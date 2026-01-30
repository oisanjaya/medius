#include "widgets/svgbox_widget.hh"
#include "gtkmm/drawingarea.h"
#include "gtkmm/object.h"
#include "helper/globals.hh"
#include "helper/tinyexpr.h"
#include "kdlpp.h"
#include "sigc++/functors/mem_fun.h"
#include "widgets/base_widget.hh"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fmt/base.h>
#include <fstream>
#include <librsvg/rsvg.h>
#include <spdlog/spdlog.h>
#include <string>

namespace widgets {

double
evalExpr(std::string expr,
         std::vector<helper::VarTuple> vars,
         bool exit_on_error = true)
{
    double result;
    std::unique_ptr<te_variable[]> var_arr(new te_variable[vars.size()]);

    int var_el_idx = 0;
    int vars_idx = 0;
    for (auto var_el : vars) {
        if (!std::get<0>(var_el).starts_with("@")) {
            var_arr[var_el_idx].name = std::get<0>(vars[vars_idx]).c_str();
            var_arr[var_el_idx].address = &std::get<2>(vars[vars_idx]);
            var_arr[var_el_idx].context = 0;
            var_arr[var_el_idx].type = 0;
            var_el_idx++;
        }
        vars_idx++;
    }

    int err;
    /* Compile the expression with variables. */
    te_expr* texpr = te_compile(expr.c_str(), var_arr.get(), var_el_idx, &err);

    if (texpr) {
        result = te_eval(texpr);

        te_free(texpr);
    } else {
        if (exit_on_error) {
            spdlog::error("Parse error at {} with expression {}", err, expr);
            std::exit(-1);
        } else {
            spdlog::warn("Parse error at {} with expression {}", err, expr);
            return NAN;
        }
    }
    return result;
}

std::string
SvgBox::evalSvgAttr(std::string cmd_attr_name, std::string cmd_attr_value)
{
    if (cmd_attr_name.substr(0, 1) == "@") {
        cmd_attr_name = cmd_attr_name.substr(1);

        bool is_all_vars_numbers = true;
        bool force_replace_vars = false;
        for (auto vars : *variables_) {
            if ((cmd_attr_value.find(std::get<0>(vars)) != std::string::npos)) {
                is_all_vars_numbers =
                  is_all_vars_numbers &&
                  (std::get<3>(vars) == helper::VarType::NUMBER);
            }
        }

        if (is_all_vars_numbers) {
            auto attr_val = evalExpr(cmd_attr_value, *variables_, false);

            if (isnan(attr_val)) {
                is_all_vars_numbers = false;
                force_replace_vars = true;
            } else {
                return (cmd_attr_name + "=\"" +
                        std::to_string(std::lround(attr_val)) + "\"");
            }
        }

        if (!is_all_vars_numbers) {
            std::string var_replace_str = cmd_attr_value;
            for (auto vars : *variables_) {
                if (((std::get<3>(vars) == helper::VarType::STRING ||
                      force_replace_vars)) &&
                    (cmd_attr_value.find(std::get<0>(vars)) !=
                     std::string::npos)) {

                    std::string replacing_str =
                      force_replace_vars ? std::to_string((std::get<2>(vars)))
                                         : std::get<1>(vars);
                    var_replace_str = helper::replaceString(
                      var_replace_str, std::get<0>(vars), replacing_str);
                }
            }

            return (cmd_attr_name + "=\"" + var_replace_str + "\"");
        }
    }

    return (cmd_attr_name + "=\"" + cmd_attr_value + "\"");
}

std::string
SvgBox::evalSvgAttr(std::string cmd_attr_name, kdl::Value cmd_attr_value)
{
    if (cmd_attr_name.substr(0, 1) == "@") {
        std::string cmd_arg = reinterpret_cast<const char*>(
          cmd_attr_value.as<std::u8string>().c_str());

        return evalSvgAttr(cmd_attr_name, cmd_arg);
    } else {
        std::string attr_val_str;

        if (cmd_attr_value.type() == kdl::Type::Number) {
            attr_val_str = std::to_string(cmd_attr_value.as<double>());
        } else {
            attr_val_str = reinterpret_cast<const char*>(
              cmd_attr_value.as<std::u8string>().c_str());
        }

        return (cmd_attr_name + "=\"" + attr_val_str + "\"");
    }
}

std::unique_ptr<std::vector<std::string>>
SvgBox::tranverseCommandChild(kdl::Node cmdNode)
{
    auto svg_cmds = std::make_unique<std::vector<std::string>>();

    for (auto cmd_child : cmdNode.children()) {
        std::string svg_el =
          reinterpret_cast<const char*>(cmd_child.name().c_str());

        if (svg_el == "group") {
            auto group_result = tranverseCommandChild(cmd_child);
            auto group_attr = std::make_unique<std::vector<std::string>>();

            for (auto group_arg : cmd_child.properties()) {
                group_attr->push_back(evalSvgAttr(
                  reinterpret_cast<const char*>(group_arg.first.c_str()),
                  group_arg.second));
            }

            std::string el_str = "<g ";
            for (auto el_attr : *group_attr) {
                el_str += el_attr + " ";
            }
            el_str += ">\n";
            svg_cmds->push_back(el_str);
            svg_cmds->insert(
              svg_cmds->end(), group_result->begin(), group_result->end());
            svg_cmds->push_back("</g>\n");

        } else {
            auto el_attr_vector = std::make_unique<std::vector<std::string>>();

            for (auto cmd_attr : cmd_child.children()) {
                if (svg_el == "path" && cmd_attr.name() == u8"data") {

                    std::string path_data_str = "d=\"";

                    for (auto path_data : cmd_attr.children()) {

                        std::string concat_args = "";
                        bool is_relative = false;

                        for (auto args : path_data.args()) {

                            if (args.type() == kdl::Type::Number) {
                                concat_args +=
                                  std::to_string(args.as<int>()) + " ";
                            } else {
                                std::string args_value =
                                  reinterpret_cast<const char*>(
                                    (args.as<std::u8string>() + u8" ").c_str());

                                if (helper::trim(args_value) == "relative") {
                                    is_relative = true;
                                } else {
                                    concat_args += args_value;
                                }
                            }
                        }

                        auto evald_data = evalSvgAttr(
                          path_data.name().substr(0, 1) == u8"@" ? "@" : "",
                          concat_args);
                        evald_data =
                          helper::replaceString(evald_data, "=\"", " ");
                        evald_data =
                          helper::replaceString(evald_data, "\"", "");

                        if (path_data.name().substr(0, 1) == u8"@") {
                            path_data.set_name(path_data.name().substr(1));
                        }

                        char svg_cmd_char = 0;

                        if (path_data.name() == u8"moveto") {
                            svg_cmd_char = 'm';
                        } else if (path_data.name() == u8"lineto") {
                            svg_cmd_char = 'l';
                        } else if (path_data.name() == u8"curveto") {
                            svg_cmd_char = 'c';
                        } else if (path_data.name() == u8"smoothcurveto") {
                            svg_cmd_char = 's';
                        } else if (path_data.name() == u8"quadraticto") {
                            svg_cmd_char = 'q';
                        } else if (path_data.name() == u8"smoothquadraticto") {
                            svg_cmd_char = 't';
                        } else if (path_data.name() == u8"arc") {
                            svg_cmd_char = 'a';
                        } else if (path_data.name() == u8"closepath") {
                            svg_cmd_char = 'z';
                        }

                        if (!is_relative) {
                            svg_cmd_char = std::toupper(svg_cmd_char);
                        }
                        path_data_str += svg_cmd_char + evald_data;
                    }

                    path_data_str += "\" ";
                    el_attr_vector->push_back(path_data_str);
                } else {
                    el_attr_vector->push_back(evalSvgAttr(
                      reinterpret_cast<const char*>(cmd_attr.name().c_str()),
                      cmd_attr.args()[0]));
                }
            }

            std::string el_str = "<" + svg_el + " ";
            for (auto el_attr : *el_attr_vector) {
                el_str += el_attr + " ";
            }
            el_str += "></" + svg_el + ">\n";
            svg_cmds->push_back(el_str);
        }
    }

    return svg_cmds;
}

SvgBox::SvgBox(config::RowItem* row_item_parent, const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    variables_ = std::make_unique<std::vector<helper::VarTuple>>();
    auto svg_cmds = std::make_unique<std::vector<std::string>>();

    std::string appended_svg = "";
    auto vpwidth = -1;
    auto vpheight = -1;
    auto vbwidth = -1;
    auto vbheight = -1;

    for (kdl::Node child : node_data.children()) {

        if (child.name() == u8"variable") {

            for (auto var_node : child.children()) {
                std::string var_name =
                  reinterpret_cast<const char*>(var_node.name().c_str());

                if (var_name.substr(0, 1) == "$") {
                    std::string shell_cmd_output =
                      helper::executeCommand(reinterpret_cast<const char*>(
                        var_node.args()[0].as<std::u8string>().c_str()));

                    if (helper::isNumber(shell_cmd_output)) {
                        variables_->emplace_back(
                          var_name.substr(1),
                          "",
                          std::atoi(shell_cmd_output.c_str()),
                          helper::VarType::NUMBER);
                    } else {
                        variables_->emplace_back(var_name.substr(1),
                                                 shell_cmd_output,
                                                 -1,
                                                 helper::VarType::STRING);
                    }

                } else if (var_name.substr(0, 1) != "@") {
                    if (var_node.args()[0].type() == kdl::Type::Number) {
                        variables_->emplace_back(
                          var_name,
                          "",
                          var_node.args()[0].as<double>(),
                          helper::VarType::NUMBER);
                    } else {
                        variables_->emplace_back(
                          var_name,
                          reinterpret_cast<const char*>(
                            var_node.args()[0].as<std::u8string>().c_str()),
                          -1,
                          helper::VarType::STRING);
                    }

                } else {
                    variables_->emplace_back(
                      var_name,
                      reinterpret_cast<const char*>(
                        var_node.args()[0].as<std::u8string>().c_str()),
                      -1,
                      helper::VarType::UNRESOLVED);
                }
            }

            int resolve_try_count = 0;
            bool is_var_resolved = false;
            while (!is_var_resolved && resolve_try_count++ < 100) {
                int variables_idx = 0;
                for (auto var_data : *variables_) {
                    if (std::get<0>(var_data).starts_with("@")) {
                        auto var_value =
                          evalExpr(std::get<1>(var_data), *variables_);
                        std::get<0>(var_data) = std::get<0>(var_data).substr(1);
                        std::get<1>(var_data) = "";
                        std::get<2>(var_data) = var_value;
                        std::get<3>(var_data) = helper::VarType::NUMBER;
                        (*variables_)[variables_idx] = var_data;
                    }
                    variables_idx++;
                }
                is_var_resolved = true;
                for (auto var_data : *variables_) {
                    is_var_resolved = is_var_resolved &&
                                      !std::get<0>(var_data).starts_with("@");
                }
            }

            if (resolve_try_count >= 100) {
                spdlog::error("SvgboxWidget failed expanding variables, most "
                              "likely due to circular reference");
                std::exit(-1);
            }
        } else if (child.name() == u8"command") {

            auto cmd_result = tranverseCommandChild(child);
            svg_cmds->insert(
              svg_cmds->end(), cmd_result->begin(), cmd_result->end());

        } else if (child.name() == u8"height") {
            vpheight = child.args()[0].as<int>();
        } else if (child.name() == u8"embed") {
            std::string filename = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
            std::ifstream file(filename);
            if (!file.is_open()) {
                spdlog::warn(
                  "Svg error: Could not open file {}, ignoring embed file.",
                  filename);
            } else {
                std::string file_content;
                std::string line;
                while (std::getline(file, line)) {
                    file_content += line + " ";
                }
                auto svg_tag_pos = file_content.find("<svg");
                if (svg_tag_pos != std::string::npos) {
                    file_content = file_content.substr(svg_tag_pos);
                }
                svg_cmds->push_back(
                  std::move(file_content)); // Move to avoid unnecessary copies
            }

        } else if (child.name() == u8"width") {
            vpwidth = child.args()[0].as<int>();
        } else if (child.name() == u8"viewbox_height") {
            vbheight = child.args()[0].as<int>();
        } else if (child.name() == u8"viewbox_width") {
            vbwidth = child.args()[0].as<int>();
        }
    }

    svg_string_ = "<svg {0} {1} viewBox=\"0 0 {2} {3}\"\n"
                  "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

    svg_string_ = fmt::format(
      fmt::runtime(svg_string_),
      vpheight > 0 ? ("height=\"" + std::to_string(vpheight) + "\"") : "",
      vpwidth > 0 ? ("width=\"" + std::to_string(vpwidth) + "\"") : "",
      vbwidth > 0 ? std::to_string(vbwidth) : "100",
      vbheight > 0 ? std::to_string(vbheight) : "100");

    for (auto svg_cmd : *svg_cmds) {
        svg_string_ += svg_cmd;
    }

    svg_string_ += "</svg>";

    spdlog::debug("{}", svg_string_);

    widget_type_ = "SvgBox";
    widget_ = Gtk::make_managed<Gtk::DrawingArea>();
    auto drawingarea_widget = static_cast<Gtk::DrawingArea*>(widget_);
    drawingarea_widget->set_size_request(vpwidth, vpheight);

    GError* error = nullptr;
    svg_handle_ = rsvg_handle_new_from_data(
      reinterpret_cast<const guint8*>(svg_string_.c_str()),
      svg_string_.size(),
      &error);

    if (error) {
        spdlog::error("Error loading SVG: {}", error->message);
        spdlog::debug("svg string:\n{}", svg_string_);
        g_error_free(error);
        std::exit(-1);
    }

    drawingarea_widget->set_draw_func(
      sigc::mem_fun(*this, &SvgBox::on_drawingarea_draw));
}

void
SvgBox::on_drawingarea_draw(const Cairo::RefPtr<Cairo::Context>& cr,
                            int width,
                            int height)
{
    RsvgRectangle viewport = { 0, 0, (double)400, (double)400 };
    rsvg_handle_render_document(svg_handle_, cr->cobj(), &viewport, nullptr);
}

SvgBox::~SvgBox() {}
}