#pragma once

#include "glibmm/dispatcher.h"
#include "gtkmm/widget.h"
#include "kdlpp.h"
#include "rotated_widget.hh"

#include <string>

namespace config {
class RowItem;
}

namespace widgets {

class BaseWidget
{
  private:
    std::string tooltip_;
    int tooltip_interval_{ -1 };
    bool dynamic_tooltip_{ false };
    std::string tooltip_result_;
    mutable std::mutex mtx_tooltip_;
    Glib::Dispatcher tooltip_dispatcher_;
    sigc::connection tooltip_dispatcher_connection_;

  protected:
    std::string label_;
    std::string label_no_space_;
    WidgetRotation rotation_{ NORMAL };

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
    void setTooltip();
    std::string getTooltip();
    void regenerateTooltip();
    bool isRegenerateTooltipBusy();
};

}