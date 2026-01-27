#include "textbox_widget.hh"
#include "gtkmm/box.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
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
    widget_ = Gtk::make_managed<Gtk::Box>();
    auto textbox_box = static_cast<Gtk::Box*>(widget_);

    textbox_box->add_css_class("medius-textbox-box");
    textbox_box->add_css_class("medius-textbox-box_" + label_no_space_);
    textbox_box->set_name("medius-textbox-box_" + label_no_space_);
    textbox_box->set_orientation(Gtk::Orientation::HORIZONTAL);
    textbox_box->set_valign(Gtk::Align::FILL);
    textbox_box->set_halign(Gtk::Align::FILL);

    auto textbox_label = Gtk::make_managed<Gtk::Label>();
    textbox_label->set_markup(label_);
    textbox_box->append(*textbox_label);
}

TextBox::~TextBox() {}

}