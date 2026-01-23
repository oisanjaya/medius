#pragma once

#include "gtkmm/cssprovider.h"
#include "gtkmm/widget.h"
#include <string>
namespace helper {

class WidgetCssProvider
{
    std::string initial_format_;
    std::string format_data_;
    Glib::RefPtr<Gtk::CssProvider> provider_;
  public:
    WidgetCssProvider(const std::string initial_format);
    
    void append(std::string css_string);
    void applyToContext(Gtk::Widget* widget);
};

}