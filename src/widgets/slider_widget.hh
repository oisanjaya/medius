#pragma once

#include "glibmm/dispatcher.h"
#include "gtkmm/button.h"
#include "gtkmm/popovermenu.h"
#include "gtkmm/scale.h"
#include "kdlpp.h"
#include "sigc++/connection.h"
#include "widgets/base_widget.hh"
#include "widgets/button_widget.hh"
#include <string>

namespace widgets {

class SliderWidget : public BaseWidget
{
    std::string icon_on_{ "none" };
    std::string icon_off_{ "none" };
    std::string on_change_;
    std::string get_state_;
    std::string on_click_on_;
    std::string on_click_off_;
    std::string state_result_;
    std::string popover_generate_result_;
    mutable std::mutex mtx_get_state_;
    Glib::Dispatcher get_state_dispatcher_;
    sigc::connection get_state_dispatcher_connection_;
    mutable std::mutex mtx_get_popover_;
    Glib::Dispatcher get_popover_dispatcher_;
    sigc::connection get_popover_dispatcher_connection_;
    std::string popover_menu_generate_;
    std::string popover_menu_on_click_;
    int icon_size_;
    int range_low_;
    int range_high_;
    kdl::Node popover_menu_node_;

    Gtk::Scale* scale_widget_;
    Gtk::PopoverMenu* popover_menu_;

    ButtonWidget* slider_button_ = nullptr;
    Gtk::Button* popover_button_ = nullptr;
    sigc::connection change_timeout_;

  public:
    SliderWidget(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~SliderWidget();

    const std::string getOnChange(void);
    const std::string getGetState(void);
    const std::string getIconOn(void);
    const std::string getIconOff(void);
    const std::string onClickOn();
    const std::string onClickOff();

    int getIconSize(void);
    int getRangeLow(void);
    int getRangeHigh(void);
    int getScaleValue(void);

    void onChange();
};

}