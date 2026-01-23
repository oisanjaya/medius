#pragma once

#include "gtkmm/cssprovider.h"
#include "widgets/base_widget.hh"
namespace widgets {

class ImageWidget : public BaseWidget
{
    std::string on_click_on_;
    std::string get_state_;
    std::string title_;
    std::string parent_row_class_;
    int get_state_interval_{ -1 };
    int title_interval_{ -1 };
    bool dynamic_get_state_{ false };
    bool dynamic_title_{ false };
    Glib::RefPtr<Gtk::CssProvider> css_provider_;

    void regenerateImage();
    void regenerateLabel();

  public:
    ImageWidget(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~ImageWidget();
};

}