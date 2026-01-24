#include <spdlog/spdlog.h>

#include "config/row_item.hh"
#include "gdkmm/display.h"
#include "glibmm/main.h"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "gtkmm/styleprovider.h"
#include "helper/globals.hh"
#include "pangomm/layout.h"
#include "widgets/base_widget.hh"
#include "widgets/image_widget.hh"

namespace widgets {

ImageWidget::ImageWidget(config::RowItem* row_item_parent,
                         const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"on_click_on") {
            on_click_on_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"get_state") {
            std::tie(dynamic_get_state_, get_state_, get_state_interval_) =
              helper::staticOrDynamicCommand(child);
        } else if (child.name() == u8"label") {
            std::tie(dynamic_title_, title_, title_interval_) =
              helper::staticOrDynamicCommand(child);
            label_ = "";
        }
    }

    std::string file_name{ "" };
    if (get_state_.length() > 0) {
        if (dynamic_get_state_) {
            file_name = helper::executeCommand(get_state_);
        } else {
            file_name = get_state_;
        }
    }

    widget_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

    Gtk::Box img_box;
    img_box.add_css_class("medius-image-box");
    img_box.set_name("medius-image-box");
    img_box.set_valign(Gtk::Align::FILL);
    img_box.set_halign(Gtk::Align::FILL);
    img_box.set_expand();
    static_cast<Gtk::Box*>(widget_)->append(img_box);

    std::string set_title{ "" };
    if ((title_.length() > 0) && (!is_label_hidden_)) {
        if (dynamic_title_) {
            set_title = helper::executeCommand(title_);
        } else {
            set_title = title_;
        }

        Gtk::Label title{ set_title };
        title.set_ellipsize(Pango::EllipsizeMode::END);
        title.set_hexpand(false);
        title.set_halign(Gtk::Align::FILL);
        title.set_valign(Gtk::Align::END);
        title.set_size_request(helper::main_config.getPanelWidth(), -1);
        title.set_max_width_chars(5);
        title.set_size_request(50, -1);
        static_cast<Gtk::Box*>(widget_)->append(title);
    }

    for (auto classname : row_item_parent_->getRowBox()->get_css_classes()) {
        if (classname.substr(0, 15) == "medius-row-box_") {
            parent_row_class_ = classname;
        }
    }

    css_provider_ = Gtk::CssProvider::create();
    css_provider_->load_from_string(
      "." + parent_row_class_ +
      " .medius-image-box { background-image: url('" + file_name +
      "');background-position: center; "
      "background-repeat:no-repeat;background-size:contain; }");

    Gtk::StyleProvider::add_provider_for_display(
      Gdk::Display::get_default(),
      css_provider_,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

    if (get_state_interval_ > 0) {
        Glib::signal_timeout().connect_seconds(
          [this]() -> bool {
              regenerateImage();
              return true;
          },
          get_state_interval_);
    }

    if (title_interval_ > 0) {
        Glib::signal_timeout().connect_seconds(
          [this]() -> bool {
              regenerateLabel();
              return true;
          },
          title_interval_);
    }
}
ImageWidget::~ImageWidget() {}

void
ImageWidget::regenerateLabel()
{
    std::string set_title{ "" };
    if ((title_.length() > 0) && (!is_label_hidden_)) {
        if (dynamic_title_) {
            set_title = helper::executeCommand(title_);
        } else {
            set_title = title_;
        }

        Gtk::Label* label_widget =
          static_cast<Gtk::Label*>(widget_->get_last_child());
        label_widget->set_text(set_title);
    }
}

void
ImageWidget::regenerateImage()
{
    std::string file_name{ "" };
    if (get_state_.length() > 0) {
        if (dynamic_get_state_) {
            file_name = helper::executeCommand(get_state_);
        } else {
            file_name = get_state_;
        }
    }

    Gtk::StyleProvider::remove_provider_for_display(Gdk::Display::get_default(),
                                                    css_provider_);
    css_provider_->load_from_string(
      "." + parent_row_class_ +
      " .medius-image-box { background-image: url('" + file_name +
      "');background-position: center; "
      "background-repeat:no-repeat;background-size:contain; }");

    Gtk::StyleProvider::add_provider_for_display(
      Gdk::Display::get_default(),
      css_provider_,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
}

}