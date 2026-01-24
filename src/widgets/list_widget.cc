#include "widgets/list_widget.hh"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/enums.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "helper/globals.hh"
#include <ranges>
#include <spdlog/spdlog.h>
#include <string>
namespace widgets {

const std::string
ListWidget::onClickOn(void)
{
    if (on_click_on_.length() <= 0) {
        return onClickOff();
    }
    return on_click_on_;
}

const std::string
ListWidget::onClickOff(void)
{
    return on_click_off_;
}

ListWidget::ListWidget(config::RowItem* row_item_parent,
                       const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"generate") {
            generate_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"on_click_on") {
            if (child.args()[0].as<std::u8string>() == u8"regenerate_list") {
                regenerate_list_ = true;
                if (child.args().size() >= 2) {
                    on_click_on_ = reinterpret_cast<const char*>(
                      child.args()[1].as<std::u8string>().c_str());
                } else {
                    on_click_on_ = "";
                }
            } else {
                on_click_on_ = reinterpret_cast<const char*>(
                  child.args()[0].as<std::u8string>().c_str());
            }
        } else if (child.name() == u8"on_click_off") {
            if (child.args()[0].as<std::u8string>() == u8"regenerate_list") {
                regenerate_list_ = true;
                if (child.args().size() >= 2) {
                    on_click_off_ = reinterpret_cast<const char*>(
                      child.args()[1].as<std::u8string>().c_str());
                } else {
                    on_click_off_ = "";
                }
            } else {
                on_click_off_ = reinterpret_cast<const char*>(
                  child.args()[0].as<std::u8string>().c_str());
            }
        } else if (child.name() == u8"hover_color") {
            hover_color_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        }
    }

    if (hover_color_.length() > 0) {
        css_provider_ = Gtk::CssProvider::create();
        css_provider_->load_from_string(".medius-list-box_" +
                                        label_no_space_ +
                                        " .medius-list-item-box:hover {"
                                        "background-color: " +
                                        hover_color_ + ";}");

        Gtk::StyleProvider::add_provider_for_display(
          Gdk::Display::get_default(),
          css_provider_,
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    regenerate(true);
};

ListWidget::~ListWidget()
{
    if (widget_) {
        delete widget_;
    }
};

void
ListWidget::regenerate(bool first_run)
{
    if (widget_) {
        for (auto box_child : static_cast<Gtk::Box*>(widget_)->get_children()) {
            box_child->unparent();
        }
    } else {
        widget_ = new Gtk::Box(Gtk::Orientation::VERTICAL, 0);
        widget_->add_css_class("medius-list-box");
        widget_->add_css_class("medius-list-box_" + label_no_space_);
        widget_->set_name("medius-list-box_" + label_no_space_);
    }

    static_cast<Gtk::Box*>(widget_)->set_homogeneous();
    widget_type_ = "List";

    // on regenerate_dispatcher_ emision, which means command has been
    // completed, run regenerate_done() to update UI
    regenerate_dispatcher_connection_ =
      regenerate_dispatcher_.connect([this]() {
          std::lock_guard<std::mutex> lock(mtx_generate_result_);
          row_item_parent_->setSpinner(false);
          regenerate_done(generate_result);
          // just to be safe, disconnect dispatcher
          regenerate_dispatcher_connection_.disconnect();
      });

    // run command on different thread; upon completion emit
    // regenerate_dispathcer_
    std::thread([this]() {
        std::lock_guard<std::mutex> lock(mtx_generate_result_);
        row_item_parent_->setSpinner(true);
        generate_result = helper::executeCommand(generate_, false);
        regenerate_dispatcher_.emit();
    }).detach();
}

void
ListWidget::regenerate_done(std::string generate_result)
{
    spdlog::debug("List widget; `generate` result: \n{}", generate_result);

    if (generate_result.length() > 0) {
        list_data_.clear();
    }

    auto result_lines = generate_result | std::views::split('\n');
    for (auto&& line : result_lines) {
        std::string_view result_line_sv{ line.begin(), line.end() };

        if (!std::ranges::all_of(result_line_sv, [](unsigned char ch) {
                return std::isspace(ch);
            })) {
            auto fields_view = result_line_sv | std::views::split('\t');
            std::vector<std::string> fields;
            for (auto&& field : fields_view) {
                fields.emplace_back(field.begin(), field.end());
            }

            if ((fields.size() == 3) &&
                ((fields[2] == "1") || (fields[2] == "0"))) {
                list_data_.emplace_back(fields[0], fields[1], fields[2]);
            } else {
                spdlog::warn(
                  "Config file error: generate command of list widget "
                  "output doesn't conform regular pattern:\n{}",
                  result_line_sv);
                spdlog::debug("filed.size(): {}", fields.size());
                spdlog::debug("fileds[0]: {}", fields.size() > 0 ? fields[0] : "no field");
                spdlog::debug("fileds[1]: {}", fields.size() > 1 ? fields[1] : "no field");
                spdlog::debug("fileds[2]: {}", fields.size() > 2 ? fields[2] : "no field");
                return;
            }
        }
    }

    for (auto list_datum : list_data_) {
        Gtk::Box list_item_box{ Gtk::Orientation::HORIZONTAL, 0 };

        list_item_box.add_css_class("medius-list-item-box");
        list_item_box.set_name("medius-list-item-box");
        list_item_box.set_margin(helper::main_config.getPanelPadding());

        auto list_item_label =
          Gtk::make_managed<Gtk::Label>(std::get<1>(list_datum));
        list_item_label->set_hexpand();
        list_item_label->set_justify(Gtk::Justification::LEFT);
        list_item_label->add_css_class("medius-list-item-label");
        list_item_label->set_name("medius-list-item-label");
        list_item_box.append(*list_item_label);

        auto list_item_button =
          Gtk::make_managed<Gtk::Button>(reinterpret_cast<const char*>(
            std::get<2>(list_datum) == "1" ? u8"✓" : u8"-"));
        list_item_button->signal_clicked().connect([this, list_datum]() {
            spdlog::debug("click list {}", std::get<2>(list_datum));
            std::string but_cmd = "";
            if (std::get<2>(list_datum) == "1") {
                if (this->onClickOn().length() > 0) {
                    but_cmd = this->onClickOn();
                }
            } else {
                if (this->onClickOff().length() > 0) {
                    but_cmd = this->onClickOff();
                }
            }

            if (but_cmd.length() > 0) {
                spdlog::debug("click list but_cmd {}", but_cmd);

                std::string value = std::get<0>(list_datum);
                value = value.substr(value.find_first_not_of(" \n\r\t\f\v"));
                value =
                  value.substr(0, value.find_last_not_of(" \n\r\t\f\v") + 1);

                auto cmd_string = helper::replaceString(
                  but_cmd.substr(1), but_cmd.substr(0, 1), value);

                spdlog::debug("click list cmd {}", cmd_string);

                list_click_dispatcher_connection_ =
                  list_click_dispatcher_.connect([this]() {
                      std::lock_guard<std::mutex> lock(mtx_list_click_);
                      row_item_parent_->setSpinner(false);
                      if (regenerate_list_) {
                          row_item_parent_->regenerateList();
                      }
                      list_click_dispatcher_connection_.disconnect();
                  });

                std::thread([this, cmd_string]() {
                    std::lock_guard<std::mutex> lock(mtx_list_click_);
                    row_item_parent_->setSpinner(true);
                    helper::executeCommand(cmd_string);
                    list_click_dispatcher_.emit();
                }).detach();
            }
        });
        list_item_button->add_css_class("medius-list-item-button");
        list_item_button->set_name("medius-list-item-button");
        list_item_button->set_size_request(32, -1);
        list_item_box.append(*list_item_button);

        static_cast<Gtk::Box*>(widget_)->append(list_item_box);
    }
}

}