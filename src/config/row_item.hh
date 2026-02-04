#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "gtkmm/box.h"
#include "gtkmm/overlay.h"
#include "gtkmm/revealer.h"
#include "gtkmm/spinner.h"
#include "kdlpp.h"
#include "widgets/base_widget.hh"
#include "widgets/expander_item.hh"

namespace config {

class RowItem
{
    Gtk::Box* row_box_;
    Gtk::Box* expander_box_;
    RowItem* parent_row_item_;

    int height_{ -1 };
    std::u8string name_;
    std::string enabled_;
    int enabled_interval_{ -1 };
    bool dynamic_enabled_{ false };
    std::vector<std::shared_ptr<RowItem>> nested_rows_;
    std::vector<std::shared_ptr<widgets::BaseWidget>> widgets_;
    std::vector<std::shared_ptr<RowItem>>* row_item_storage_;
    Gtk::Revealer* revealer_;
    Gtk::Overlay* overlay_;
    Gtk::Spinner* spinner_{ nullptr };
    bool expander_busy_;

    void reevaluateEnabled();

  public:
    RowItem(Gtk::Box* parent_box,
            const kdl::Node& node_data,
            std::vector<std::shared_ptr<RowItem>>* row_item_storage = nullptr,
            RowItem* parent_row_item = nullptr);
    ~RowItem();

    std::string getName();
    std::shared_ptr<widgets::BaseWidget> getWidget(size_t idx);
    Gtk::Box* getRowBox();
    bool isExpanderBusy();
    size_t getWidgetCount();
    int getRowHeight();
    void toggleExpander(widgets::ExpanderItem* expander_item);
    void regenerateList();
    void setSpinner(bool spinning);
};

}