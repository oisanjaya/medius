#pragma once
#include "widgets/base_widget.hh"
#include <gtkmm.h>

namespace medius {

class MainWindow : public Gtk::ApplicationWindow
{
    sigc::connection close_timer_;

    void connectGtkButtonSignals(widgets::BaseWidget* widget);
    void connectGtkToggleButtonSignals(widgets::BaseWidget* widget);

    void on_mouse_enter(double x, double y);
    void on_mouse_leave();

  protected:
    void on_map() override;

  public:
    MainWindow();
    ~MainWindow() override;
};

}