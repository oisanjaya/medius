#include "textbox_widget.hh"
#include "glibmm/main.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "helper/globals.hh"
#include "widgets/base_widget.hh"

namespace widgets {

TextBox::TextBox(config::RowItem* row_item_parent, const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"command") {
            command_ = reinterpret_cast<const char*>(
              child.args()[0].as<std::u8string>().c_str());
        } else if (child.name() == u8"label") {
            std::tie(dynamic_text_label_, text_label_, text_label_interval_) =
              helper::staticOrDynamicCommand(child);

            if (dynamic_text_label_) {
                Glib::signal_timeout().connect_seconds(
                  [this]() -> bool {
                      regenerateLabel();
                      return true;
                  },
                  text_label_interval_);
            }
        }
    }

    widget_type_ = "TextBox";
    auto textbox_box = Gtk::make_managed<Gtk::Box>();

    textbox_box->add_css_class("medius-textbox-box");
    textbox_box->add_css_class("medius-textbox-box_" + label_no_space_);
    textbox_box->set_name("medius-textbox-box_" + label_no_space_);
    textbox_box->set_orientation(Gtk::Orientation::HORIZONTAL);
    textbox_box->set_valign(Gtk::Align::FILL);
    textbox_box->set_halign(Gtk::Align::FILL);

    textbox_label_ = Gtk::make_managed<Gtk::Label>();
    textbox_label_->set_markup(label_);
    textbox_box->append(*textbox_label_);

    if (command_.empty()) {
        widget_ = textbox_box;
    } else {
        widget_ = Gtk::make_managed<Gtk::Button>();
        auto button_widget = reinterpret_cast<Gtk::Button*>(widget_);
        button_widget->set_has_frame(false);
        button_widget->set_child(*textbox_box);
        button_widget->signal_clicked().connect(
          [this]() { helper::executeCommand(command_); });
    }
    setTooltip(tooltip_);
}

TextBox::~TextBox() {}

void TextBox::regenerateLabel() {
    auto text_label_result = helper::executeCommand(text_label_);
    textbox_label_->set_markup(text_label_result);
}

}