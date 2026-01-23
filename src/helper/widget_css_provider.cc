#include "widget_css_provider.hh"
#include "gtkmm/widget.h"
#include <fmt/base.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <string>

namespace helper {

WidgetCssProvider::WidgetCssProvider(std::string initial_format)
{
    provider_ = Gtk::CssProvider::create();
    initial_format_ = initial_format;
}

void
WidgetCssProvider::append(std::string css_string)
{
    format_data_ += css_string;
}

void
WidgetCssProvider::applyToContext(Gtk::Widget* widget)
{
    auto css_data_ = fmt::format(fmt::runtime(initial_format_), format_data_);

    provider_->load_from_data(css_data_);
    auto widget_style_context = widget->get_style_context();

    widget_style_context->add_provider(provider_,
                                       GTK_STYLE_PROVIDER_PRIORITY_USER);
}

}