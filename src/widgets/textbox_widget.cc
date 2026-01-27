#include "textbox_widget.hh"
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

    auto textbox_label = Gtk::make_managed<Gtk::Label>();
    textbox_label->set_markup(label_);
    textbox_box->append(*textbox_label);

    if (command_.empty()) {
        widget_ = textbox_box;
    } else {
        widget_ = Gtk::make_managed<Gtk::Button>();
        auto button_widget = reinterpret_cast<Gtk::Button*>(widget_);
        button_widget->set_has_frame(false);
        button_widget->set_child(*textbox_box);
        button_widget->signal_clicked().connect([this](){
            helper::executeCommand(command_);
        });
    }
}

TextBox::~TextBox() {}

}