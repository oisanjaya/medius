#include "widgets/base_widget.hh"
#include "config/row_item.hh"
#include "gtkmm/widget.h"
#include "kdlpp.h"
#include <cctype>
#include <spdlog/spdlog.h>
#include <string>

namespace widgets {

BaseWidget::BaseWidget(config::RowItem* row_item_parent,
                       const kdl::Node& node_data)
  : row_item_parent_(row_item_parent)
{
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"label") {
            for (auto arg : child.args()) {
                if (arg.as<std::u8string>() == u8"hidden") {
                    is_label_hidden_ = true;
                } else {
                    label_ = reinterpret_cast<const char*>(
                      arg.as<std::u8string>().c_str());
                    label_no_space_ = label_;
                    std::replace(
                      label_no_space_.begin(), label_no_space_.end(), ' ', '_');
                    std::transform(
                      label_no_space_.begin(),
                      label_no_space_.end(),
                      label_no_space_.begin(),
                      [](unsigned char c) { return std::tolower(c); });
                }
            }
        }
    }
}

BaseWidget::~BaseWidget() {}

const std::string
BaseWidget::getLabel(void)
{
    return label_;
}

Gtk::Widget*
BaseWidget::getWidget(void)
{
    return widget_;
}

const std::string
BaseWidget::getWidgetType(void)
{
    return widget_type_;
}

}