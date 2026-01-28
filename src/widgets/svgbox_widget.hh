#pragma once

#include "config/row_item.hh"
#include "librsvg/rsvg.h"
#include "widgets/base_widget.hh"
namespace widgets {

class SvgBox : public BaseWidget
{
    RsvgHandle* svg_handle_{ nullptr };
    std::string svg_string_;

    void on_drawingarea_draw(const Cairo::RefPtr<Cairo::Context>& cr,
                             int width,
                             int height);

  public:
    SvgBox(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~SvgBox();
};

}