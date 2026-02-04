#pragma once

#include "gtkmm/box.h"
#include "kdlpp.h"
#include "widgets/base_widget.hh"

namespace widgets {
class ExpanderItem
{
    Gtk::Box* expander_box_;

    std::u8string name_;
    bool expanded_{ false };
    kdl::Node node_data_;
    std::vector<std::shared_ptr<config::RowItem>> children_;
    config::RowItem* row_item_parent_;

  public:
    ExpanderItem(config::RowItem* row_item_parent, kdl::Node node_data);
    ~ExpanderItem();

    Gtk::Box* getExpanderBox();

    std::u8string name();
    bool getExpanded();
};

}