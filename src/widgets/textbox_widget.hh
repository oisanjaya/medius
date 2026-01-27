#pragma once

#include "widgets/base_widget.hh"
#include <string>

namespace widgets {

class TextBox : public BaseWidget
{
    std::string command_;

  public:
    TextBox(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~TextBox();
};

}