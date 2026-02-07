#pragma once

#include "gtkmm/widget.h"

namespace widgets {

enum WidgetRotation {NORMAL, RIGHT, UPSIDEDOWN, LEFT};

class RotatedBin : public Gtk::Widget
{
    Gtk::Widget* child = nullptr;
    WidgetRotation rotation_{ NORMAL };

  public:
    RotatedBin(const WidgetRotation rotation = NORMAL);
    ~RotatedBin();
    void set_child(Gtk::Widget& w);
    void measure_vfunc(Gtk::Orientation orientation,
                       int for_size,
                       int& minimum,
                       int& natural,
                       int& minimum_baseline,
                       int& natural_baseline) const override;
    void size_allocate_vfunc(int width, int height, int baseline) override;
    void snapshot_vfunc(const Glib::RefPtr<Gtk::Snapshot>& snapshot) override;
};

}