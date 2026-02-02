#pragma once

#include "gtkmm/label.h"
#include "widgets/base_widget.hh"
#include <string>

namespace widgets {

class TextBox : public BaseWidget
{
    std::string command_;
    bool dynamic_text_label_;
    std::string text_label_;
    int text_label_interval_;
    Gtk::Label* textbox_label_{ nullptr };

    void regenerateLabel();

  public:
    TextBox(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~TextBox();
};

}