#include "window.hh"
#include "config/config.hh"
#include "gdkmm/display.h"
#include "gtk/gtk.h"
#include "gtk4-layer-shell/gtk4-layer-shell.h"
#include "gtkmm/box.h"
#include "gtkmm/cssprovider.h"
#include "gtkmm/enums.h"
#include "gtkmm/eventcontrollerfocus.h"
#include "gtkmm/scale.h"
#include "gtkmm/styleprovider.h"
#include "helper/globals.hh"

#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <sys/types.h>

#include <gtkmm.h>
#include <librsvg/rsvg.h>
namespace medius {

Gtk::Scale*
find_slider_widget(Gtk::Box* scale_box_widget)
{
    for (auto child : scale_box_widget->get_children()) {
        if (typeid(*child) == typeid(Gtk::Scale)) {
            return static_cast<Gtk::Scale*>(child);
        }
        if (typeid(*child) == typeid(Gtk::Box)) {
            auto recurse_result =
              find_slider_widget(static_cast<Gtk::Box*>(child));
            if (recurse_result) {
                return recurse_result;
            }
        }
    }
    return nullptr;
};

MainWindow::MainWindow()
{
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource("/id/my/oisanjaya/medius/medius.ui");
    } catch (const Glib::Error& ex) {
        spdlog::error("Error loading UI: {}", ex.what());
        std::exit(-1);
    }

    auto main_css_provider = Gtk::CssProvider::create();
    try {
        main_css_provider->load_from_resource(
          "/id/my/oisanjaya/medius/medius.css");
    } catch (const Glib::Error& ex) {
        spdlog::error("Error loading style: {}", ex.what());
        std::exit(-1);
    }

    Gtk::StyleProvider::add_provider_for_display(
      Gdk::Display::get_default(),
      main_css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWindow* main_window = GTK_WINDOW(this->gobj());

    gtk_layer_init_for_window(main_window);
    gtk_layer_set_layer(main_window, GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_keyboard_mode(main_window,
                                GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
    Gtk::Box* main_box = builder->get_widget<Gtk::Box>("main_box");
    helper::main_config.load(main_box, helper::cli_config.config_opt);

    set_default_size(helper::main_config.getPanelWidth(), -1);
    set_size_request(helper::main_config.getPanelWidth(), -1);

    main_box->add_css_class("medius-main-box");
    main_box->set_name("medius-main-box");
    main_box->set_margin_top(helper::main_config.getPanelPadding());
    main_box->set_margin_start(helper::main_config.getPanelPadding());
    main_box->set_margin_end(helper::main_config.getPanelPadding());
    main_box->set_halign(Gtk::Align::FILL);
    main_box->set_valign(Gtk::Align::START);

    if (helper::main_config.getAnchorLeft()) {
        gtk_layer_set_anchor(main_window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_margin(main_window,
                             GTK_LAYER_SHELL_EDGE_LEFT,
                             helper::main_config.getPanelMargin());
    }
    if (helper::main_config.getAnchorRight()) {
        gtk_layer_set_anchor(main_window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
        gtk_layer_set_margin(main_window,
                             GTK_LAYER_SHELL_EDGE_RIGHT,
                             helper::main_config.getPanelMargin());
    }
    if (helper::main_config.getAnchorTop()) {
        gtk_layer_set_anchor(main_window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
        gtk_layer_set_margin(main_window,
                             GTK_LAYER_SHELL_EDGE_TOP,
                             helper::main_config.getPanelMargin());
    }
    if (helper::main_config.getAnchorBottom()) {
        gtk_layer_set_anchor(main_window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
        gtk_layer_set_margin(main_window,
                             GTK_LAYER_SHELL_EDGE_BOTTOM,
                             helper::main_config.getPanelMargin());
    }

    if (main_box) {
        main_box->set_spacing(helper::main_config.getChildSpacing());

        set_child(*main_box);
    }

    helper::latter_css_data.append(R"(:root {{
    --rounded-corner: {rounded_corner}px;
}}
)");

    helper::latter_css_data = fmt::format(
      fmt::runtime(helper::latter_css_data),
      fmt::arg("rounded_corner", helper::main_config.getRoundedCorner()));

    spdlog::debug("passed css file {}", helper::cli_config.config_opt);

    auto filename = helper::cli_config.css_opt.empty()
                      ? helper::main_config.findConfigPath({ "style.css" })
                      : helper::cli_config.css_opt;

    if (filename.has_value()) {
        spdlog::debug("Using additional CSS file: {}", filename.value());
        std::ifstream file(filename.value(), std::ios::in);
        if (!file) {
            spdlog::warn("Loading CSS file failed!");
        } else {
            std::ostringstream buffer;
            buffer << file.rdbuf();
            helper::latter_css_data += buffer.str();
        }
    }

    spdlog::debug("latter_css_data: {}", helper::latter_css_data);

    auto root_css_provider = Gtk::CssProvider::create();
    root_css_provider->load_from_string(helper::latter_css_data);

    Gtk::StyleProvider::add_provider_for_display(
      Gdk::Display::get_default(),
      root_css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

    if (helper::main_config.getCloseTimeout() > 0) {
        auto focus_ctrl = Gtk::EventControllerFocus::create();

        focus_ctrl->signal_leave().connect([this]() { setCloseTimeout(); });
        focus_ctrl->signal_enter().connect([this]() { cancelCloseTimeout(); });

        add_controller(focus_ctrl);
    }
}

void
MainWindow::cancelCloseTimeout()
{
    if (close_timer_.connected()) {
        spdlog::debug("Close Timer cancelled.");
        close_timer_.disconnect();
    }
}

void
MainWindow::setCloseTimeout()
{
    if (helper::disable_lost_focus_quit) {
        spdlog::debug("Close Timer prevented.");
        return;
    }

    spdlog::debug("Window will closed in {}",
                  helper::main_config.getCloseTimeout());
    close_timer_ = Glib::signal_timeout().connect(
      [this]() {
          this->close();
          return false;
      },
      helper::main_config.getCloseTimeout());
}

void
MainWindow::on_map()
{
    // Call the base class implementation first
    Gtk::Window::on_map();
}

MainWindow::~MainWindow() {}

}