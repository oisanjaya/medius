#include "gtkmm/button.h"
#include "helper/globals.hh"
#include "widgets/base_widget.hh"
#include <ranges>
#include <spdlog/spdlog.h>
#include <string>

#include "popover_widget.hh"

namespace widgets {

PopoverWidget::PopoverWidget(const kdl::Node& node_data)
  : BaseWidget(nullptr, node_data)
{
    widget_ = Gtk::make_managed<Gtk::Button>(">");
    auto button_widget_ = static_cast<Gtk::Button*>(widget_);
    button_widget_->add_css_class("medius-slider-popover-button");
    button_widget_->set_name("medius-slider-popover-button");
    button_widget_->set_size_request(10, -1);

    for (kdl::Node popover_menu_child : node_data.children()) {
        if (popover_menu_child.name() == u8"generate") {
            popover_menu_generate_ = reinterpret_cast<const char*>(
              popover_menu_child.args()[0].as<std::u8string>().c_str());
        } else if (popover_menu_child.name() == u8"on_click") {
            popover_menu_on_click_ = reinterpret_cast<const char*>(
              popover_menu_child.args()[0].as<std::u8string>().c_str());
        }
    }

    button_widget_->signal_clicked().connect([this]() {
        helper::disable_lost_focus_quit = true;
        if (popover_menu_generate_.length() > 0) {
            popover_menu_ = Gtk::make_managed<Gtk::PopoverMenu>();

            popover_menu_->signal_closed().connect([this]() {
                helper::disable_lost_focus_quit = false;
                popover_menu_->unparent();
            });

            get_popover_dispatcher_connection_ =
              get_popover_dispatcher_.connect([this]() {
                  auto button_widget_ = static_cast<Gtk::Button*>(widget_);
                  std::lock_guard<std::mutex> lock(mtx_get_popover_);
                  auto popover_menu_box =
                    Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

                  spdlog::debug(
                    "Slider widget popover; `generate` result: \n{}",
                    popover_generate_result_);

                  if (popover_generate_result_.length() > 0) {
                      generateResult(popover_menu_box);
                  }

                  popover_menu_->set_child(*popover_menu_box);
                  popover_menu_->set_parent(
                    *static_cast<Gtk::Widget*>(button_widget_));
                  popover_menu_->set_has_arrow(false);

                  popover_menu_->popup();

                  get_popover_dispatcher_connection_.disconnect();
              });

            std::thread([this]() {
                std::lock_guard<std::mutex> lock(mtx_get_popover_);
                popover_generate_result_ =
                  helper::executeCommand(popover_menu_generate_, false);
                get_popover_dispatcher_.emit();
            }).detach();
        }
    });
}

PopoverWidget::~PopoverWidget() {}

void
PopoverWidget::generateResult(Gtk::Box* popover_menu_box)
{
    std::vector<PopoverWidgetTuple> list_data_;

    auto result_lines = popover_generate_result_ | std::views::split('\n');
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

            if (fields.size() == 2) {
                list_data_.emplace_back(fields[0], fields[1]);
            } else {
                spdlog::warn("Config file error: generate "
                             "command of popover widget "
                             "output doesn't conform regular "
                             "pattern:\n{}",
                             result_line_sv);
                spdlog::debug("filed.size(): {}", fields.size());
                spdlog::debug("fileds[0]: {}",
                              fields.size() > 0 ? fields[0] : "no field");
                spdlog::debug("fileds[1]: {}",
                              fields.size() > 1 ? fields[1] : "no field");
                return;
            }
        }
    }

    for (auto list_datum : list_data_) {
        auto popover_menu_item =
          Gtk::make_managed<Gtk::Button>(std::get<1>(list_datum));
        popover_menu_item->set_has_frame(false);
        popover_menu_box->append(*popover_menu_item);

        auto menu_command =
          helper::replaceString(popover_menu_on_click_.substr(1),
                                popover_menu_on_click_.substr(0, 1),
                                std::get<0>(list_datum));

        popover_menu_item->signal_clicked().connect([this, menu_command]() {
            helper::executeCommand(menu_command, false);
            popover_menu_->popdown();
        });
    }
}

}