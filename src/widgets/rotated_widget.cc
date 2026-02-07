#include "rotated_widget.hh"
#include "gtkmm/enums.h"
#include "gtkmm/snapshot.h"

namespace widgets {

RotatedBin::RotatedBin(WidgetRotation rotation)
  : Gtk::Widget()
  , rotation_(rotation)
{
}

RotatedBin::~RotatedBin() {
    if (child) {
        child->unparent();
    }
}

void
RotatedBin::set_child(Gtk::Widget& w)
{
    child = &w;
    child->set_parent(*this);
}

void
RotatedBin::measure_vfunc(Gtk::Orientation orientation,
                          int for_size,
                          int& minimum,
                          int& natural,
                          int& minimum_baseline,
                          int& natural_baseline) const
{
    if (!child)
        return;

    // if parent asks for size and rotation_ is sideways, forward to child as
    // swapped widht/height
    Gtk::Orientation child_orient =
      ((rotation_ == LEFT) || (rotation_ == RIGHT))
        ? ((orientation == Gtk::Orientation::HORIZONTAL)
             ? (Gtk::Orientation::VERTICAL)
             : Gtk::Orientation::HORIZONTAL)
        : orientation;

    child->measure(child_orient,
                   for_size,
                   minimum,
                   natural,
                   minimum_baseline,
                   natural_baseline);
}

void
RotatedBin::size_allocate_vfunc(int width, int height, int baseline)
{
    if (child) {
        if ((rotation_ == LEFT) || (rotation_ == RIGHT)) {
            // allocate child height and width swapped off container size
            child->size_allocate(Gtk::Allocation(0, 0, height, width),
                                 baseline);
        } else {
            // for horizontal don't swap
            child->size_allocate(Gtk::Allocation(0, 0, width, height),
                                 baseline);
        }
    }
}

void
RotatedBin::snapshot_vfunc(const Glib::RefPtr<Gtk::Snapshot>& snapshot)
{
    if (!child || !child->get_visible())
        return;

    snapshot->save();

    if (rotation_ == RIGHT) {
        // Translate right by the container's width
        snapshot->translate(Gdk::Graphene::Point((float)get_width(), 0.0f));
        snapshot->rotate(90.0f);
    } else if (rotation_ == LEFT) {
        // Translate down by the container's width
        snapshot->translate(Gdk::Graphene::Point(0.0f, (float)get_height()));
        snapshot->rotate(-90.0f);
    } else if (rotation_ == UPSIDEDOWN) {
        // Translate right and down by the container's width
        snapshot->translate(
          Gdk::Graphene::Point((float)get_width(), (float)get_height()));
        snapshot->rotate(180.0f);
    }

    // Draw the child at its allocated size
    Gdk::Graphene::Rect child_rect(
      0, 0, (float)child->get_width(), (float)child->get_height());
    snapshot_child(*child, snapshot);

    snapshot->restore();
}

}