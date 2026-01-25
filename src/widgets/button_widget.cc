#include "button_widget.hh"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/enums.h"
#include "gtkmm/image.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "gtkmm/togglebutton.h"
#include "helper/globals.hh"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include "widgets/expander_item.hh"
#include <spdlog/spdlog.h>
#include <string>

namespace widgets {

ButtonWidget::ButtonWidget(config::RowItem* row_item_parent,
                           const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"on_click_on") {
            if (child.args()[0].as<std::u8string>() == u8"regenerate_list") {
                regenerate_list_ = true;
                if (child.args().size() > 2) {
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
            on_click_off_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"expander") {
            expander_item_ = new ExpanderItem(row_item_parent, child);
        } else if (child.name() == u8"get_state") {
            std::tie(dynamic_get_state_, get_state_, get_state_interval_) =
              helper::staticOrDynamicCommand(child);
        } else if (child.name() == u8"icon_on") {
            icon_on_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"icon_off") {
            icon_off_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"icon_size") {
            icon_size_ = child.args()[0].as<int>();
        } else if (child.name() == u8"regenerate_list") {
            regenerate_list_ = true;
        }
    }

    Gtk::Box but_box;
    but_box.add_css_class("medius-button-label-box");
    but_box.set_name("medius-button-label-box");
    but_box.set_orientation(Gtk::Orientation::VERTICAL);
    but_box.set_valign(Gtk::Align::CENTER);
    but_box.set_halign(Gtk::Align::CENTER);

    but_img_ = Gtk::make_managed<Gtk::Image>();
    but_img_->add_css_class("medius-button-image");
    but_img_->set_name("medius-button-image");
    if (icon_size_ > 0) {
        but_img_->set_pixel_size(icon_size_);
    }

    if (icon_off_ == "none") {
        but_img_->set_from_icon_name(icon_on_);
    } else {
        but_img_->set_from_icon_name(icon_off_);
    }

    if (but_img_->get_icon_name() == "none") {
        but_img_->hide();
    } else {
        but_img_->show();
    }
    but_box.append(*but_img_);

    Gtk::Label but_lbl(label_);
    but_lbl.add_css_class("medius-button-label");
    but_lbl.set_name("medius-button-label");
    if (is_label_hidden_) {
        but_lbl.hide();
    } else {
        but_lbl.show();
    }
    but_box.append(but_lbl);

    if ((on_click_on_.length() > 0) && (on_click_off_.length() > 0)) {
        but_widget_ = Gtk::make_managed<Gtk::ToggleButton>();
        if (expander_item_) {
            but_widget_->add_css_class("medius-button-has-expander");
        } else {
            but_widget_->add_css_class("medius-button");
        }
        but_widget_->add_css_class("medius-button_" + label_no_space_);
        but_widget_->set_name("medius-button_" + label_no_space_);
        connectGtkToggleButtonSignals();
        widget_type_ = "ToggleButton";
    } else {
        but_widget_ = Gtk::make_managed<Gtk::Button>();
        if (expander_item_) {
            but_widget_->add_css_class("medius-button-has-expander");
        } else {
            but_widget_->add_css_class("medius-button");
        }
        but_widget_->add_css_class("medius-button_" + label_no_space_);
        but_widget_->set_name("medius-button_" + label_no_space_);
        connectGtkButtonSignals();
        widget_type_ = "Button";
    }

    but_widget_->set_child(but_box);
    but_widget_->set_hexpand();
    but_widget_->set_hexpand(true);
    but_widget_->set_halign(Gtk::Align::FILL);

    regenerateState();

    if (expander_item_) {
        widget_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        auto button_group_box = static_cast<Gtk::Box*>(widget_);
        button_group_box->add_css_class("medius-row-expander-button-box");
        button_group_box->set_name("medius-row-expander-button-box");
        button_group_box->set_spacing(0);
        button_group_box->append(*but_widget_);

        Gtk::Button* expander_button = Gtk::make_managed<Gtk::Button>(">");
        expander_button->add_css_class("medius-row-expander-button");
        expander_button->set_name("medius-row-expander-button");
        expander_button->set_size_request(10, -1);
        expander_button->set_halign(Gtk::Align::END);
        expander_button->set_valign(Gtk::Align::FILL);
        expander_button->set_vexpand(true);
        expander_button->set_hexpand(false);

        expander_button->signal_clicked().connect([this, row_item_parent]() {
            if (row_item_parent) {
                row_item_parent->toggleExpander(expander_item_);
            }
        });

        button_group_box->append(*expander_button);
        button_group_box->set_hexpand(true);
        button_group_box->set_halign(Gtk::Align::FILL);
    } else {
        widget_ = but_widget_;
    }

    regenerateState();
    if (get_state_interval_ > 0) {
        Glib::signal_timeout().connect_seconds(
          [this]() -> bool {
              regenerateState();
              return true;
          },
          get_state_interval_);
    }
}

ButtonWidget::~ButtonWidget() {}

void
ButtonWidget::connectGtkButtonSignals()
{
    Gtk::Button* button_widget = static_cast<Gtk::Button*>(but_widget_);
    button_widget->signal_clicked().connect([this]() {
        if (this->onClickOn().length() > 0) {
            helper::executeCommand(this->onClickOn());
        }

        regenerateState();

        if (regenerate_list_) {
            if (row_item_parent_) {
                row_item_parent_->regenerateList();
            }
        }
    });
}

void
ButtonWidget::connectGtkToggleButtonSignals()
{
    Gtk::ToggleButton* button_widget =
      static_cast<Gtk::ToggleButton*>(but_widget_);
    button_widget->signal_toggled().connect([this, button_widget]() {
        if (button_widget->get_active()) {
            helper::executeCommand(this->onClickOn());
        } else {
            helper::executeCommand(this->onClickOff());
        }

        regenerateState();

        if (regenerate_list_) {
            if (row_item_parent_) {
                row_item_parent_->regenerateList();
            }
        }
    });
}

void
ButtonWidget::regenerateState()
{
    if (get_state_.length() > 0) {
        get_state_dispatcher_connection_ =
          get_state_dispatcher_.connect([this]() {
              std::lock_guard<std::mutex> lock(mtx_get_state_);

              int state_number = std::stoi(state_result_);
              bool but_active = state_number > 0;

              setActive(but_active);

              get_state_dispatcher_connection_.disconnect();
          });

        std::thread([this]() {
            std::lock_guard<std::mutex> lock(mtx_get_state_);
            state_result_ = helper::executeCommand(get_state_);
            get_state_dispatcher_.emit();
        }).detach();
    }
}

void
ButtonWidget::setActive(bool active)
{
    if (typeid(*but_widget_) == typeid(Gtk::ToggleButton)) {
        static_cast<Gtk::ToggleButton*>(but_widget_)->set_active(active);
    }

    if (active) {
        but_img_->set_from_icon_name(icon_on_);
    } else {
        but_img_->set_from_icon_name(icon_off_);
    }

    if (but_img_->get_icon_name() == "none") {
        but_img_->hide();
    } else {
        but_img_->show();
    }
}

const std::string
ButtonWidget::getOnClickOn(void)
{
    return on_click_on_;
}

const std::string
ButtonWidget::getOnClickOff(void)
{
    return on_click_off_;
}

const std::string
ButtonWidget::getGetState(void)
{
    return get_state_;
}

const std::string
ButtonWidget::getIconOn(void)
{
    return icon_on_;
}

const std::string
ButtonWidget::getIconOff(void)
{
    return icon_off_;
}

int
ButtonWidget::getIconSize(void)
{
    return icon_size_;
}

const std::string
ButtonWidget::onClickOn(void)
{
    if (on_click_on_.length() <= 0) {
        return onClickOff();
    }
    return on_click_on_;
}

const std::string
ButtonWidget::onClickOff(void)
{
    return on_click_off_;
}

bool
ButtonWidget::GetRegenerateList(void)
{
    return regenerate_list_;
}

}