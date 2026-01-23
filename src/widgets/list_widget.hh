#pragma once

#include "glibmm/dispatcher.h"
#include "gtkmm/cssprovider.h"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include <string>

namespace widgets {

using ListWidgetTuple = std::tuple<std::string, std::string, std::string>;

class ListWidget : public BaseWidget
{
    std::string generate_;
    std::string generate_result = "";
    mutable std::mutex
      mtx_generate_result_;             // Mutex to protect 'generate_result_'
    mutable std::mutex mtx_list_click_; // Mutex to protect 'list_click_'
    std::string on_click_on_;
    std::string on_click_off_;
    std::string hover_color_{ "" };
    Glib::RefPtr<Gtk::CssProvider> css_provider_;
    std::vector<ListWidgetTuple> list_data_;
    Glib::Dispatcher regenerate_dispatcher_;
    sigc::connection regenerate_dispatcher_connection_;
    Glib::Dispatcher list_click_dispatcher_;
    sigc::connection list_click_dispatcher_connection_;
    bool regenerate_list_{ false };

    void connectGtkButtonSignals();
    void connectGtkToggleButtonSignals();
    void regenerate_done(std::string generate_result);

  public:
    ListWidget(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~ListWidget();

    void regenerate(bool first_run = false);

    const std::string onClickOn(void);
    const std::string onClickOff(void);
};

}