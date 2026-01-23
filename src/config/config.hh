#pragma once

#include "config/row_item.hh"
#include "kdlpp.h"
#include <optional>
#include <string>
#include <vector>

namespace config {

class Config
{
    int width_, margin_, spacing_, animation_duration_, rounded_corner_;
    int padding_{ DEFAULT_PANEL_PADDING };
    int close_timeout_{ -1 };
    std::string list_item_box_hover_color_{ DEFAULT_LIST_ITEM_BOX_HOVER_COLOR };
    std::string config_file_;
    kdl::Document config_;
    std::vector<RowItem*> rows_;

  public:
    static const std::vector<std::string> CONFIG_DIRS;
    static const char* CONFIG_PATH_ENV;

    /* Try to find any of provided names in the supported set of config
     * directories */
    static std::optional<std::string> findConfigPath(
      const std::vector<std::string>& names,
      const std::vector<std::string>& dirs = CONFIG_DIRS);

    static std::vector<std::string> tryExpandPath(const std::string& base,
                                                  const std::string& filename);
    Config();
    ~Config();

    void load(Gtk::Box* parent_box, const std::string& config);

    size_t getRowSize();
    std::optional<RowItem*> getRow(const std::string& name);
    std::optional<RowItem*> getRow(size_t index);
    int getPanelWidth();
    int getPanelMargin();
    int getPanelPadding();
    int getChildSpacing();
    int getAnimationDuration();
    int getRoundedCorner();
    int getCloseTimeout();
    const std::string getListItemBoxHoverColor();
};
} // namespace config
