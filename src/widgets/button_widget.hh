#pragma once

#include "expander_item.hh"
#include "glibmm/dispatcher.h"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include <string>

namespace widgets {

class ButtonWidget : public BaseWidget
{
    std::string on_click_on_;
    std::string on_click_off_;
    std::string get_state_;
    std::string icon_on_{ "none" };
    std::string icon_off_{ "none" };
    std::string state_result_;
    mutable std::mutex mtx_get_state_;
    Glib::Dispatcher get_state_dispatcher_;
    sigc::connection get_state_dispatcher_connection_;
    int icon_size_{ -1 };
    bool regenerate_list_{ false };

    ExpanderItem* expander_item_{ nullptr };

    void connectGtkButtonSignals();
    void connectGtkToggleButtonSignals();

  public:
    ButtonWidget(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~ButtonWidget();

    const std::string getOnClickOn(void);
    const std::string getOnClickOff(void);
    const std::string getGetState(void);
    const std::string getIconOn(void);
    const std::string getIconOff(void);
    bool GetRegenerateList();

    int getIconSize(void);

    const std::string onClickOn(void);
    const std::string onClickOff(void);
};

}