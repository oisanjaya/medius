#include "row_item.hh"
#include "glibmm/main.h"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "gtkmm/object.h"
#include "gtkmm/overlay.h"
#include "gtkmm/revealer.h"
#include "helper/globals.hh"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include "widgets/button_widget.hh"
#include "widgets/expander_item.hh"
#include "widgets/image_widget.hh"
#include "widgets/list_widget.hh"
#include "widgets/slider_widget.hh"
#include <cstddef>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace config {

RowItem::RowItem(Gtk::Box* parent_box,
                 const kdl::Node& node_data,
                 std::vector<config::RowItem*>* row_item_storage,
                 RowItem* parent_row_item)
  : parent_row_item_(parent_row_item)
  , row_item_storage_(row_item_storage)
{
    name_ = node_data.name();
    expander_busy_ = false;

    row_box_ = Gtk::make_managed<Gtk::Box>();
    row_box_->set_spacing(helper::main_config.getChildSpacing());
    row_box_->set_homogeneous();
    row_box_->set_halign(Gtk::Align::FILL);
    row_box_->set_valign(Gtk::Align::FILL);

    if (!parent_row_item_) {
        auto overlay_box = Gtk::make_managed<Gtk::Box>(
          Gtk::Orientation::VERTICAL, helper::main_config.getChildSpacing());
        expander_box_ = Gtk::make_managed<Gtk::Box>();
        expander_box_->add_css_class("medius-row-box");
        expander_box_->add_css_class(
          std::string("medius-row-box_") +
          reinterpret_cast<const char*>(name_.c_str()));
        expander_box_->set_name(std::string("medius-row-box_") +
                                reinterpret_cast<const char*>(name_.c_str()));

        overlay_box->add_css_class("medius-row-expander-box");
        overlay_box->set_name("medius-row-expander-box");

        revealer_ = new Gtk::Revealer();
        overlay_box->add_css_class("medius-row-expander");
        overlay_box->set_name("medius-row-expander");
        revealer_->set_transition_type(Gtk::RevealerTransitionType::SLIDE_DOWN);
        revealer_->set_transition_duration(
          helper::main_config.getAnimationDuration());
        revealer_->set_valign(Gtk::Align::START);

        overlay_box->append(*row_box_);
        overlay_box->append(*revealer_);

        spinner_ = Gtk::make_managed<Gtk::Spinner>();
        spinner_->set_halign(Gtk::Align::CENTER);
        spinner_->set_valign(Gtk::Align::CENTER);

        overlay_ = new Gtk::Overlay();
        overlay_->set_halign(Gtk::Align::FILL);
        overlay_->set_valign(Gtk::Align::FILL);
        overlay_->set_hexpand();
        overlay_->set_vexpand();
        overlay_->set_child(*overlay_box);
        overlay_->add_overlay(*spinner_);
        expander_box_->append(*overlay_);
    }

    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"rows") {
            auto nested_row = Gtk::make_managed<Gtk::Box>();
            nested_row->set_orientation(row_box_->get_orientation() ==
                                            Gtk::Orientation::HORIZONTAL
                                          ? Gtk::Orientation::VERTICAL
                                          : Gtk::Orientation::HORIZONTAL);
            nested_row->set_spacing(helper::main_config.getChildSpacing());
            nested_row->set_homogeneous();
            nested_row->set_halign(Gtk::Align::FILL);
            nested_row->set_valign(Gtk::Align::FILL);
            row_box_->append(*nested_row);

            for (kdl::Node row_node : child.children()) {
                spdlog::debug(
                  "nested row create {}",
                  reinterpret_cast<const char*>(row_node.name().c_str()));
                auto nested_row_item =
                  new RowItem(nested_row, row_node, &nested_rows_, this);

                nested_rows_.push_back(nested_row_item);
            }
        } else if (child.name() == u8"button") {
            auto widget = new widgets::ButtonWidget(this, child);

            widgets_.push_back(widget);
            row_box_->append(*widget->getWidget());
        } else if (child.name() == u8"slider") {
            auto widget = new widgets::SliderWidget(this, child);

            widgets_.push_back(widget);
            row_box_->append(*widget->getWidget());
        } else if (child.name() == u8"list") {
            auto widget = new widgets::ListWidget(this, child);

            widgets_.push_back(widget);
            row_box_->append(*widget->getWidget());
        } else if (child.name() == u8"image") {
            auto widget = new widgets::ImageWidget(this, child);

            widgets_.push_back(widget);
            row_box_->append(*widget->getWidget());
        } else if (child.name() == u8"height") {
            height_ = child.args()[0].as<int>();
        } else if (child.name() == u8"enabled") {
            std::tie(dynamic_enabled_, enabled_, enabled_interval_) =
              helper::staticOrDynamicCommand(child);
        }
    }

    if (height_ > 0) {
        row_box_->set_size_request(-1, height_);
    }

    if (!parent_row_item_) {
        parent_box->append(*expander_box_);
    } else {
        parent_box->append(*row_box_);
    }

    if (enabled_.length() > 0) {
        bool is_enabled{ true };
        if (dynamic_enabled_) {

            Glib::signal_timeout().connect_seconds(
              [this]() -> bool {
                  reevaluateEnabled();
                  return true;
              },
              enabled_interval_);

            is_enabled = helper::executeCommand(enabled_) == "1";
        }

        if (!is_enabled) {
            expander_box_->hide();
        }
    }
}

RowItem::~RowItem()
{
    for (auto ptr : nested_rows_) {
        delete ptr;
    }

    for (auto ptr : widgets_) {
        delete ptr;
    }

    if (revealer_) {
        delete revealer_;
    }

    if (overlay_) {
        delete overlay_;
    }
}

std::string
RowItem::getName()
{
    return reinterpret_cast<const char*>(name_.c_str());
}

widgets::BaseWidget*
RowItem::getWidget(size_t idx)
{
    return widgets_[idx];
}

size_t
RowItem::getWidgetCount()
{
    return widgets_.size();
}

int
RowItem::getRowHeight()
{
    return height_;
}

void
RowItem::toggleExpander(widgets::ExpanderItem* expander_item)
{
    if (parent_row_item_){
        parent_row_item_->toggleExpander(expander_item);
        return;
    }

    if (expander_busy_) {
        return;
    }
    expander_busy_ = true;

    if (revealer_->get_child_revealed()) {
        revealer_->set_reveal_child(false);
        Glib::signal_timeout().connect_once(
          [this]() -> void {
              revealer_->unset_child();
              expander_busy_ = false;
          },
          helper::main_config.getAnimationDuration());
    } else {
        revealer_->unset_child();
        Gtk::Box* revealer_box = expander_item->getExpanderBox();
        if (!revealer_box->has_css_class("medius-row-expander-widget-box")) {
            revealer_box->add_css_class("medius-row-expander-widget-box");
            revealer_box->set_name("medius-row-expander-widget-box");
        }

        revealer_->set_child(*revealer_box);
        revealer_->set_reveal_child(true);
        Glib::signal_timeout().connect_once(
          [this]() -> void { expander_busy_ = false; },
          helper::main_config.getAnimationDuration());
    }
}

void
RowItem::regenerateList()
{
    for (auto sibling_row_item : *row_item_storage_) {
        for (size_t wid = 0; wid < sibling_row_item->getWidgetCount(); wid++) {
            auto row_widget = sibling_row_item->getWidget(wid);
            if (row_widget->getWidgetType() == "List") {
                static_cast<widgets::ListWidget*>(row_widget)->regenerate();
                return;
            }
        }
    }
}

bool
RowItem::isExpanderBusy()
{
    return expander_busy_;
}

Gtk::Box*
RowItem::getRowBox()
{
    return expander_box_;
}

void
RowItem::setSpinner(bool spinning)
{
    if (spinner_) {
        if (spinning) {
            spinner_->start();

        } else {
            spinner_->stop();
        }
    }
}

void
RowItem::reevaluateEnabled()
{
    if (helper::executeCommand(enabled_) != "1") {
        expander_box_->hide();
    } else {
        expander_box_->show();
    }
}

}