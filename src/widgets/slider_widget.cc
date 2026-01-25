#include "slider_widget.hh"
#include "glibmm/main.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/enums.h"
#include "gtkmm/image.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "gtkmm/popovermenu.h"
#include "gtkmm/scale.h"
#include "gtkmm/togglebutton.h"
#include "gtkmm/widget.h"
#include "helper/globals.hh"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include "widgets/button_widget.hh"
#include <ranges>
#include <spdlog/spdlog.h>
#include <string>

namespace widgets {

SliderWidget::SliderWidget(config::RowItem* row_item_parent,
                           const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"icon_on") {
            icon_on_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"icon_off") {
            icon_off_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"on_click_on") {
            on_click_on_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"on_click_off") {
            on_click_off_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"get_state") {
            std::tie(dynamic_get_state_, get_state_, get_state_interval_) =
              helper::staticOrDynamicCommand(child);
        } else if (child.name() == u8"on_change") {
            on_change_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"icon_size") {
            icon_size_ = child.args()[0].as<int>();
        } else if (child.name() == u8"range_low") {
            range_low_ = child.args()[0].as<int>();
        } else if (child.name() == u8"range_high") {
            range_high_ = child.args()[0].as<int>();
        } else if (child.name() == u8"popover_menu") {
            popover_menu_node_ = child;
        }
    }

    widget_type_ = "Slider";
    widget_ = Gtk::make_managed<Gtk::Box>();
    auto slider_box = static_cast<Gtk::Box*>(widget_);

    slider_box->add_css_class("medius-slider-box");
    slider_box->add_css_class("medius-slider-box_" + label_no_space_);
    slider_box->set_name("medius-slider-box_" + label_no_space_);
    slider_box->set_orientation(Gtk::Orientation::HORIZONTAL);
    slider_box->set_valign(Gtk::Align::FILL);
    slider_box->set_halign(Gtk::Align::FILL);

    if (icon_on_ != "none") {
        std::string slider_button_node_string{
            "button {\nlabel hidden "
            "\"medius-slider-box_" +
            label_no_space_ + "\"\n" +
            (icon_off_ != "none" ? ("icon_off \"" + icon_off_ + "\"\n") : "") +
            (icon_on_ != "none" ? ("icon_on \"" + icon_on_ + "\"\n") : "") +
            (on_click_on_ != "none" ? ("on_click_on \"" + on_click_on_ + "\"\n")
                                    : "") +
            (on_click_off_ != "none"
               ? ("on_click_off \"" + on_click_off_ + "\"\n")
               : "") +
            (icon_size_ > 0 ? ("icon_size " + std::to_string(icon_size_) + "\n")
                            : "") +
            "}"
        };

        if (on_click_on_.length() + on_click_off_.length() > 0) {
            auto slider_button_node =
              kdl::parse(std::u8string(reinterpret_cast<const char8_t*>(
                                         slider_button_node_string.data()),
                                       slider_button_node_string.size()));

            slider_button_ =
              new ButtonWidget(nullptr, slider_button_node.nodes()[0]);
            Gtk::Button* but_widget =
              static_cast<Gtk::Button*>(slider_button_->getWidget());
            but_widget->add_css_class("medius-slider-button");
            but_widget->set_name("medius-slider-button");
            but_widget->set_hexpand(false);

            slider_box->append(*but_widget);
        } else {
            Gtk::Image slider_image;
            slider_image.add_css_class("medius-slider-image");
            slider_image.set_name("medius-slider-image");
            slider_image.set_from_icon_name(icon_on_);

            if (icon_on_ != "none") {
                slider_box->append(slider_image);
            }
        }
    }

    scale_widget_ = new Gtk::Scale{ Gtk::Orientation::HORIZONTAL };
    scale_widget_->add_css_class("medius-slider-scale");
    scale_widget_->set_name("medius-slider-scale");
    scale_widget_->set_range(range_low_, range_high_);
    scale_widget_->set_increments(1, 1);
    scale_widget_->set_expand(true);

    scale_widget_->signal_value_changed().connect([this]() { onChange(); });

    Gtk::Box* slider_box_with_label;
    if (!is_label_hidden_) {
        Gtk::Label slider_label{ label_ };
        slider_label.add_css_class("medius-slider_label");
        slider_label.set_name("medius-slider_label");
        slider_box_with_label = new Gtk::Box{ Gtk::Orientation::VERTICAL };
        slider_box_with_label->add_css_class("medius-slider-box-with-label");
        slider_box_with_label->set_name("medius-slider-box-with-label");
        slider_box_with_label->set_spacing(
          helper::main_config.getChildSpacing());
        slider_box_with_label->append(*scale_widget_);
        slider_box_with_label->append(slider_label);
        slider_box->append(*slider_box_with_label);
    } else {
        slider_box->append(*scale_widget_);
    }

    if (popover_menu_node_.children().size() > 0) {
        popover_button_ = Gtk::make_managed<Gtk::Button>(">");
        popover_button_->add_css_class("medius-slider-popover-button");
        popover_button_->set_name("medius-slider-popover-button");
        popover_button_->set_size_request(10, -1);

        for (kdl::Node popover_menu_child : popover_menu_node_.children()) {
            if (popover_menu_child.name() == u8"generate") {
                popover_menu_generate_ = reinterpret_cast<const char*>(
                  popover_menu_child.args()[0].as<std::u8string>().c_str());
            } else if (popover_menu_child.name() == u8"on_click") {
                popover_menu_on_click_ = reinterpret_cast<const char*>(
                  popover_menu_child.args()[0].as<std::u8string>().c_str());
            }
        }

        popover_button_->signal_clicked().connect([this]() {
            helper::disable_lost_focus_quit = true;
            if (popover_menu_generate_.length() > 0) {
                popover_menu_ = Gtk::make_managed<Gtk::PopoverMenu>();

                popover_menu_->signal_closed().connect([this]() {
                    helper::disable_lost_focus_quit = false;
                    popover_menu_->unparent();
                });

                get_popover_dispatcher_connection_ =
                  get_popover_dispatcher_.connect([this]() {
                      std::lock_guard<std::mutex> lock(mtx_get_popover_);
                      auto popover_menu_box =
                        Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

                      spdlog::debug(
                        "Slider widget popover; `generate` result: \n{}",
                        popover_generate_result_);

                      if (popover_generate_result_.length() > 0) {
                          std::istringstream stream(popover_generate_result_);
                          std::string line;
                          while (std::getline(stream, line)) {
                              auto popover_menu_item =
                                Gtk::make_managed<Gtk::Button>(line);
                              popover_menu_item->set_has_frame(false);
                              popover_menu_box->append(*popover_menu_item);

                              auto menu_command = helper::replaceString(
                                popover_menu_on_click_.substr(1),
                                popover_menu_on_click_.substr(0, 1),
                                line);

                              spdlog::debug("!{}!{}",
                                            popover_menu_on_click_,
                                            popover_menu_on_click_.substr(1));

                              popover_menu_item->signal_clicked().connect(
                                [this, menu_command]() {
                                    helper::executeCommand(menu_command);
                                    popover_menu_->popdown();
                                });
                          }
                      }

                      popover_menu_->set_child(*popover_menu_box);
                      popover_menu_->set_parent(
                        *static_cast<Gtk::Widget*>(popover_button_));
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

        slider_box->append(*popover_button_);
    }

    if (get_state_interval_ > 0) {
        Glib::signal_timeout().connect_seconds(
          [this]() -> bool {
              regenerateState();
              return true;
          },
          get_state_interval_);
    } else {
        regenerateState();
    }
}

SliderWidget::~SliderWidget() {}

void
SliderWidget::onChange()
{
    if (change_timeout_ && change_timeout_.connected()) {
        change_timeout_.disconnect();
    }

    change_timeout_ = Glib::signal_timeout().connect(
      [this]() -> bool {
          std::string value = std::to_string((int)scale_widget_->get_value());

          auto cmd_string = helper::replaceString(
            on_change_.substr(1), on_change_.substr(0, 1), value);

          std::string result = helper::executeCommand(cmd_string);
          return false;
      },
      DEFAULT_SLIDER_TIMEOUT);
}

void
SliderWidget::regenerateState()
{
    get_state_dispatcher_connection_ = get_state_dispatcher_.connect([this]() {
        std::lock_guard<std::mutex> lock(mtx_get_state_);

        auto fields_view = state_result_ | std::views::split('\t');
        std::vector<std::string> fields;
        for (auto&& field : fields_view) {
            fields.emplace_back(field.begin(), field.end());
        }

        for (auto& field : fields) {
            if (!field.empty()) {
                size_t start = 0;
                while (start < field.size() &&
                       std::isspace(static_cast<unsigned char>(field[start]))) {
                    ++start;
                }
                size_t end = field.size() - 1;
                if (field.size() > 0) {
                    while (
                      end > start &&
                      std::isspace(static_cast<unsigned char>(field[end]))) {
                        --end;
                    }
                }
                field = field.substr(start, end - start + 1);
            }
        }

        if (slider_button_) {
            Gtk::Button* but_widget =
              static_cast<Gtk::Button*>(slider_button_->getWidget());
            if ((typeid(*but_widget) == typeid(Gtk::ToggleButton)) &&
                ((fields.size() >= 2))) {
                slider_button_->setActive(fields[1] != "0");
            } else {
                spdlog::warn("Config file error: Slider {} widget get_state "
                             "output doesn't conform regular pattern:\n{}",
                             label_,
                             state_result_);
                spdlog::debug("filed.size(): {}", fields.size());
                spdlog::debug("fileds[0]: '{}'",
                              fields.size() > 0 ? fields[0] : "no field");
                spdlog::debug("fileds[1]: '{}'",
                              fields.size() > 1 ? fields[1] : "no field");
            }
        }

        bool parse_error = false;
        if ((fields.size() > 0) && (fields[0].length() > 0)) {
            try {
                scale_widget_->set_value(std::stod(fields[0]));
            } catch (...) {
                parse_error = true;
            }
        } else {
            parse_error = true;
        }

        if (parse_error) {
            spdlog::warn("Parse command error: set slider value; defaulting to "
                         "minimal value");
        }

        get_state_dispatcher_connection_.disconnect();
    });

    std::thread([this]() {
        std::lock_guard<std::mutex> lock(mtx_get_state_);
        state_result_ = helper::executeCommand(get_state_);
        get_state_dispatcher_.emit();
    }).detach();
}

const std::string
SliderWidget::getIconOn(void)
{
    return icon_on_;
}

const std::string
SliderWidget::getIconOff(void)
{
    return icon_off_;
}

const std::string
SliderWidget::getGetState(void)
{
    return get_state_;
}

int
SliderWidget::getIconSize(void)
{
    return icon_size_;
}

int
SliderWidget::getRangeLow(void)
{
    return range_low_;
}

int
SliderWidget::getRangeHigh(void)
{
    return range_high_;
}

int
SliderWidget::getScaleValue(void)
{
    return scale_widget_->get_value();
}

const std::string
SliderWidget::onClickOn(void)
{
    if (on_click_on_.length() <= 0) {
        return onClickOff();
    }
    return on_click_on_;
}

const std::string
SliderWidget::onClickOff(void)
{
    return on_click_off_;
}

}