#pragma once

#include "glibmm/dispatcher.h"
#include "gtkmm/box.h"
#include "gtkmm/popovermenu.h"
#include "widgets/base_widget.hh"
#include <mutex>

namespace widgets {

using PopoverWidgetTuple = std::tuple<std::string, std::string>;

class PopoverWidget : public BaseWidget
{
    mutable std::mutex mtx_get_popover_;
    Glib::Dispatcher get_popover_dispatcher_;
    sigc::connection get_popover_dispatcher_connection_;
    std::string popover_menu_generate_;
    std::string popover_menu_on_click_;
    std::string popover_generate_result_;

    Gtk::PopoverMenu* popover_menu_;

    void generateResult(Gtk::Box* popover_menu_box);

  public:
    PopoverWidget(const kdl::Node& node_data);
    ~PopoverWidget();
};

}