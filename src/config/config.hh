#pragma once

#include "config/row_item.hh"
#include "kdlpp.h"
#include <optional>
#include <string>
#include <vector>

namespace config {

enum LayerType {BACKGROUND, BOTTOM, TOP, OVERLAY, NORMAL};

class Config
{
    int width_, margin_, spacing_, animation_duration_, rounded_corner_;
    bool anchor_left_{ false };
    bool anchor_right_{ false };
    bool anchor_top_{ false };
    bool anchor_bottom_{ false };
    LayerType use_layer_{ TOP };
    int padding_{ DEFAULT_PANEL_PADDING };
    int close_timeout_{ -1 };
    int close_on_escape_{ -1 };
    std::string title_;
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
    bool getAnchorLeft();
    bool getAnchorRight();
    bool getAnchorTop();
    bool getAnchorBottom();
    std::string getTitle();
    const std::string getListItemBoxHoverColor();
    int getCloseOnEscape();
    LayerType getUseLayer();
};
} // namespace config
