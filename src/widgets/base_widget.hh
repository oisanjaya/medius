#pragma once

#include "gtkmm/widget.h"
#include "kdlpp.h"
#include <string>

namespace config {
class RowItem;
}

namespace widgets {

class BaseWidget
{
  protected:
    std::string label_;
    std::string tooltip_;
    std::string label_no_space_;

    std::string widget_type_;
    Gtk::Widget* widget_{ nullptr };

    config::RowItem* row_item_parent_;

    bool is_label_hidden_{ false };

  public:
    BaseWidget(config::RowItem* row_item_parent, const kdl::Node& node_data);
    virtual ~BaseWidget();

    Gtk::Widget* getWidget(void);
    const std::string getWidgetType(void);

    const std::string getLabel(void);
    void setTooltip(std::string text);
};

}