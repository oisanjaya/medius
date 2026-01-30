#pragma once

#include "config/row_item.hh"
#include "helper/globals.hh"
#include "kdlpp.h"
#include "librsvg/rsvg.h"
#include "widgets/base_widget.hh"
#include <string>
namespace widgets {

class SvgBox : public BaseWidget
{
    RsvgHandle* svg_handle_{ nullptr };
    std::string svg_string_;

    std::unique_ptr<std::vector<helper::VarTuple>> variables_;

    void on_drawingarea_draw(const Cairo::RefPtr<Cairo::Context>& cr,
                             int width,
                             int height);
    std::unique_ptr<std::vector<std::string>> tranverseCommandChild(
      kdl::Node cmdNode);
    std::string evalSvgAttr(std::string cmd_attr_name,
                            kdl::Value cmd_attr_value);
    std::string evalSvgAttr(std::string cmd_attr_name,
                            std::string cmd_attr_value);

  public:
    SvgBox(config::RowItem* row_item_parent, const kdl::Node& node_data);
    ~SvgBox();
};

}