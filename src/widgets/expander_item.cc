#include "expander_item.hh"
#include "config/row_item.hh"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace widgets {

ExpanderItem::ExpanderItem(config::RowItem* row_item_parent,
                           kdl::Node node_data)
  : name_(node_data.name())
  , node_data_(node_data)
  , row_item_parent_(row_item_parent)
{
    expander_box_ = new Gtk::Box{ Gtk::Orientation::VERTICAL };

    for (auto child : node_data_.children()) {
        children_.push_back(
          std::make_shared<config::RowItem>(expander_box_, child, &children_));
    }
}

ExpanderItem::~ExpanderItem()
{
    // delete child_row_;
    delete expander_box_;
}

std::u8string
ExpanderItem::name()
{
    return name_;
}

bool
ExpanderItem::getExpanded()
{
    return expanded_;
}

Gtk::Box*
ExpanderItem::getExpanderBox()
{
    return expander_box_;
}

}
