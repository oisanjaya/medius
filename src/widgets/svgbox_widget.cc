#include "widgets/svgbox_widget.hh"
#include "gtkmm/drawingarea.h"
#include "gtkmm/object.h"
#include "sigc++/functors/mem_fun.h"
#include "widgets/base_widget.hh"

#include <librsvg/rsvg.h>
#include <spdlog/spdlog.h>

namespace widgets {

SvgBox::SvgBox(config::RowItem* row_item_parent, const kdl::Node& node_data)
  : BaseWidget(row_item_parent, node_data)
{
    svg_string_ = "<?xml version=\"1.0\" standalone=\"no\"?>\n"
                  "<svg width=\"400\" height=\"400\" viewBox=\"0 0 400 400\"\n"
                  "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    for (kdl::Node child : node_data.children()) {
        if (child.name() == u8"command") {
            // command_ = reinterpret_cast<const char*>(
            //   child.args()[0].as<std::u8string>().c_str());
        }
    }
    svg_string_ +=
      "<svg fill=\"#000000\" stroke=\"blue\" width=\"400\" height=\"400\" viewBox=\"0 0 32 "
      "32\" xmlns=\"http://www.w3.org/2000/svg\" id=\"Layer_1\" "
      "data-name=\"Layer 1\"><path "
      "d=\"M29.22,13.95h-.28v-2.07c0-4.75-5.76-8.61-12.84-8.61S3.26,7.14,3.26,"
      "11.88v2.07h-.48c-.84,0-1.52,.68-1.52,1.52v1.06c0,.84,.68,1.52,1.52,1."
      "52h.48v2.07c0,4.74,5.76,8.6,12.84,8.6s12.84-3.86,12.84-8.6v-2.07h.28c."
      "84,0,1.52-.68,1.52-1.52v-1.06c0-.84-.68-1.52-1.52-1.52ZM16.1,4.78c5.85,"
      "0,10.68,2.79,11.28,6.36H4.82c.6-3.57,5.43-6.36,11.28-6.36ZM4.76,12."
      "63H27.44v1.32H4.76v-1.32Zm11.34,14.58c-5.85,0-10.68-2.79-11.28-6.35h12."
      "49l1.8,3c.14,.23,.38,.36,.64,.36s.51-.14,.64-.36l1.8-3h5.17c-.6,3.56-5."
      "43,6.35-11.28,6.35Zm11.34-7.85h-5.66c-.26,0-.51,.14-.64,.36l-1.38,2.29-"
      "1.38-2.29c-.14-.23-.38-.36-.64-.36H4.76v-1.32H27.44v1.32Zm1.78-2.82l-26."
      "46-.02,.02-1.08h1.22s0,0,0,0H28.19s0,0,0,0h1.02s.02,.02,.02,.02l-.02,1."
      "08Z\"/></svg>\n";
    svg_string_ += "  <path d=\"M 50 150 \n";
    svg_string_ += "A 80 80 0 0 1 250 150\" fill=\"none\" stroke=\"blue\" \n"
                   "stroke-width=\"3\"></path>\n";
    svg_string_ +=
      "<circle cx=\"50\" cy=\"150\" r=\"4\" fill=\"red\"></circle>\n";
    svg_string_ +=
      "<circle cx=\"250\" cy=\"150\" r=\"4\" fill=\"green\"></circle>\n";

    svg_string_ += "</svg>";

    widget_type_ = "SvgBox";
    widget_ = Gtk::make_managed<Gtk::DrawingArea>();
    auto drawingarea_widget = static_cast<Gtk::DrawingArea*>(widget_);
    drawingarea_widget->set_size_request(400, 400);

    GError* error = nullptr;
    spdlog::debug("output svg:\n{}", svg_string_);
    svg_handle_ = rsvg_handle_new_from_data(
      reinterpret_cast<const guint8*>(svg_string_.c_str()),
      svg_string_.size(),
      &error);

    if (error) {
        spdlog::error("Error loading SVG: {}", error->message);
        g_error_free(error);
        std::exit(-1);
    }

    drawingarea_widget->set_draw_func(
      sigc::mem_fun(*this, &SvgBox::on_drawingarea_draw));
}

void
SvgBox::on_drawingarea_draw(const Cairo::RefPtr<Cairo::Context>& cr,
                            int width,
                            int height)
{
    RsvgRectangle viewport = { 0, 0, (double)400, (double)400 };
    rsvg_handle_render_document(svg_handle_, cr->cobj(), &viewport, nullptr);
}

SvgBox::~SvgBox() {}

}