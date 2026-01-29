#include "widgets/svgbox_widget.hh"
#include "gtkmm/drawingarea.h"
#include "gtkmm/object.h"
#include "helper/exprtk.hpp"
#include "helper/globals.hh"
#include "kdlpp.h"
#include "sigc++/functors/mem_fun.h"
#include "widgets/base_widget.hh"

#include <cmath>
#include <cstdlib>
#include <fmt/base.h>
#include <librsvg/rsvg.h>
#include <spdlog/spdlog.h>
#include <string>

namespace widgets {

template<typename T>
T
evalExpr(std::string expr, std::vector<helper::VarTuple> vars)
{
    typedef exprtk::symbol_table<T> symbol_table_t;
    typedef exprtk::expression<T> expression_t;
    typedef exprtk::parser<T> parser_t;

    symbol_table_t symbol_table;

    std::unique_ptr<T[]> var_arr(new T[vars.size()]);

    int var_el_idx = 0;
    for (auto var_el : vars) {
        var_arr[var_el_idx++] = T(std::get<2>(var_el));
    }

    var_el_idx = 0;
    for (auto var_el : vars) {
        if (!std::get<0>(var_el).starts_with("@")) {
            symbol_table.add_variable(std::get<0>(var_el), var_arr[var_el_idx]);
        }
        var_el_idx++;
    }

    expression_t expression;
    expression.register_symbol_table(symbol_table);

    parser_t parser;
    parser.compile(expr, expression);

    const T result = expression.value();

    return result;
}

SvgBox::SvgBox(config::RowItem* row_item_parent, const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    var_vector_ = std::make_unique<std::vector<helper::VarTuple>>();
    auto command_vector = std::make_unique<std::vector<std::string>>();

    svg_string_ = "<svg {0} {1} viewBox=\"0 0 {2} {3}\"\n"
                  "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

    std::string appended_svg = "";
    auto vpwidth = -1;
    auto vpheight = -1;
    auto vbwidth = -1;
    auto vbheight = -1;

    for (kdl::Node child : node_data.children()) {

        if (child.name() == u8"command") {

            for (auto cmd_child : child.children()) {
                auto el_attr_vector =
                  std::make_unique<std::vector<std::string>>();
                std::string svg_el =
                  reinterpret_cast<const char*>(cmd_child.name().c_str());

                for (auto cmd_attr : cmd_child.children()) {
                    std::string cmd_attr_name =
                      reinterpret_cast<const char*>(cmd_attr.name().c_str());

                    if (cmd_attr_name.substr(0, 1) == "@") {
                        cmd_attr_name = cmd_attr_name.substr(1);

                        auto attr_val = evalExpr<double>(
                          reinterpret_cast<const char*>(
                            cmd_attr.args()[0].as<std::u8string>().c_str()),
                          *var_vector_);

                        el_attr_vector->push_back(
                          cmd_attr_name + "=\"" +
                          std::to_string(std::lround(attr_val))+"\"");
                    } else {
                        std::string attr_val_str;

                        if (cmd_attr.args()[0].type() == kdl::Type::Number) {
                            attr_val_str =
                              std::to_string(cmd_attr.args()[0].as<int>());
                        } else {
                            attr_val_str = reinterpret_cast<const char*>(
                              cmd_attr.args()[0].as<std::u8string>().c_str());
                        }

                        el_attr_vector->push_back(cmd_attr_name + "=\"" +
                                                  attr_val_str + "\"");
                    }
                }

                std::string el_str = "<" + svg_el + " ";
                for (auto el_attr : *el_attr_vector) {
                    el_str += el_attr + " ";
                }

                el_str += "></" + svg_el + ">\n";
                command_vector->push_back(el_str);
            }
        } else if (child.name() == u8"variable") {

            for (auto var_node : child.children()) {
                std::string var_name =
                  reinterpret_cast<const char*>(var_node.name().c_str());

                if (var_name.substr(0, 1) == "$") {
                    std::string shell_cmd_output =
                      helper::executeCommand(reinterpret_cast<const char*>(
                        var_node.args()[0].as<std::u8string>().c_str()));

                    var_vector_->emplace_back(
                      var_name.substr(1),
                      "",
                      std::atoi(shell_cmd_output.c_str()));

                } else if (var_name.substr(0, 1) != "@") {
                    int var_node_value = var_node.args()[0].as<int>();
                    var_vector_->emplace_back(var_name, "", var_node_value);

                } else {
                    var_vector_->emplace_back(
                      var_name,
                      reinterpret_cast<const char*>(
                        var_node.args()[0].as<std::u8string>().c_str()),
                      -1);
                }
            }

            int resolve_try_count = 0;
            bool is_var_resolved = false;
            while (!is_var_resolved && resolve_try_count++ < 100) {
                int var_vector_idx = 0;
                for (auto var_data : *var_vector_) {
                    if (std::get<0>(var_data).starts_with("@")) {
                        auto var_value =
                          evalExpr<double>(std::get<1>(var_data), *var_vector_);
                        std::get<0>(var_data) = std::get<0>(var_data).substr(1);
                        std::get<1>(var_data) = "";
                        std::get<2>(var_data) = var_value;
                        (*var_vector_)[var_vector_idx] = var_data;
                    }
                    var_vector_idx++;
                }
                is_var_resolved = true;
                for (auto var_data : *var_vector_) {
                    is_var_resolved = is_var_resolved &&
                                      !std::get<0>(var_data).starts_with("@");
                }
            }

            if (resolve_try_count >= 100) {
                spdlog::error("SvgboxWidget failed expanding variables, most "
                              "likely due to circular reference");
                std::exit(-1);
            }
        } else if (child.name() == u8"height") {
            vpheight = child.args()[0].as<int>();
        } else if (child.name() == u8"width") {
            vpwidth = child.args()[0].as<int>();
        } else if (child.name() == u8"viewbox_height") {
            vbheight = child.args()[0].as<int>();
        } else if (child.name() == u8"viewbox_width") {
            vbwidth = child.args()[0].as<int>();
        }
    }

    svg_string_ = fmt::format(
      fmt::runtime(svg_string_),
      vpheight > 0 ? ("height=\"" + std::to_string(vpheight) + "\"") : "",
      vpwidth > 0 ? ("width=\"" + std::to_string(vpwidth) + "\"") : "",
      vbwidth > 0 ? std::to_string(vbwidth) : "100",
      vbheight > 0 ? std::to_string(vbheight) : "100");

    // svg_string_ +=
    //   "<svg fill=\"#000000\" stroke=\"blue\" width=\"50\" height=\"50\"\n"
    //   "viewBox=\"0 0 32 " "32\" xmlns=\"http://www.w3.org/2000/svg\"\n"
    //   "id=\"Layer_1\" " "data-name=\"Layer 1\"><path \n"
    //   "d=\"M29.22,13.95h-.28v-2.07c0-4.75-5.76-8.61-12.84-8.61S3.26,7.14,3.26,\n"
    //   "11.88v2.07h-.48c-.84,0-1.52,.68-1.52,1.52v1.06c0,.84,.68,1.52,1.52,1.\n"
    //   "52h.48v2.07c0,4.74,5.76,8.6,12.84,8.6s12.84-3.86,12.84-8.6v-2.07h.28c.\n"
    //   "84,0,1.52-.68,1.52-1.52v-1.06c0-.84-.68-1.52-1.52-1.52ZM16.1,4.78c5.85,\n"
    //   "0,10.68,2.79,11.28,6.36H4.82c.6-3.57,5.43-6.36,11.28-6.36ZM4.76,12.\n"
    //   "63H27.44v1.32H4.76v-1.32Zm11.34,14.58c-5.85,0-10.68-2.79-11.28-6.35h12.\n"
    //   "49l1.8,3c.14,.23,.38,.36,.64,.36s.51-.14,.64-.36l1.8-3h5.17c-.6,3.56-5.\n"
    //   "43,6.35-11.28,6.35Zm11.34-7.85h-5.66c-.26,0-.51,.14-.64,.36l-1.38,2.29-\n"
    //   "1.38-2.29c-.14-.23-.38-.36-.64-.36H4.76v-1.32H27.44v1.32Zm1.78-2.82l-26.\n"
    //   "46-.02,.02-1.08h1.22s0,0,0,0H28.19s0,0,0,0h1.02s.02,.02,.02,.02l-.02,1.\n"
    //   "08Z\"/>\n</svg>\n";
    // svg_string_ += "  <path d=\"M 50 150 \n";
    // svg_string_ += "A 80 80 0 0 1 250 150\" fill=\"none\" stroke=\"blue\" \n"
    //                "stroke-width=\"3\"></path>\n";
    // svg_string_ +=
    //   "<circle cx=\"50\" cy=\"50\" r=\"4\" fill=\"red\"></circle>\n";
    // svg_string_ +=
    //   "<circle cx=\"60\" cy=\"60\" r=\"4\" fill=\"green\"></circle>\n";

    for (auto svg_cmd : *command_vector) {
        svg_string_ += svg_cmd;
    }

    svg_string_ += "</svg>";

    widget_type_ = "SvgBox";
    widget_ = Gtk::make_managed<Gtk::DrawingArea>();
    auto drawingarea_widget = static_cast<Gtk::DrawingArea*>(widget_);
    drawingarea_widget->set_size_request(400, 400);

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