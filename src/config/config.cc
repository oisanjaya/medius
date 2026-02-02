#include "config.hh"
#include "config/row_item.hh"
#include "gtkmm/box.h"
#include "kdlpp.h"

#include <cstddef>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/types.h>
#include <wordexp.h>

namespace fs = std::filesystem;

namespace config {

const std::vector<std::string> Config::CONFIG_DIRS = {
    "$XDG_CONFIG_HOME/medius/", "$HOME/.config/medius/", "$HOME/medius/",
    "/etc/xdg/medius/",         "./resources/",
};

const char* Config::CONFIG_PATH_ENV = "MEDIUS_CONFIG_DIR";

Config::Config() {}

Config::~Config()
{
    for (auto row_item : rows_) {
        if (row_item) {
            delete row_item;
        }
    }
}

std::vector<std::string>
Config::tryExpandPath(const std::string& base, const std::string& filename)
{
    fs::path path;

    if (!filename.empty()) {
        path = fs::path(base) / fs::path(filename);
    } else {
        path = fs::path(base);
    }

    std::vector<std::string> results;

    wordexp_t p;
    if (wordexp(path.c_str(), &p, 0) == 0) {
        for (size_t i = 0; i < p.we_wordc; i++) {
            if (access(p.we_wordv[i], F_OK) == 0) {
                results.emplace_back(p.we_wordv[i]);
            }
        }
        wordfree(&p);
    }

    return results;
}

std::optional<std::string>
Config::findConfigPath(const std::vector<std::string>& names,
                       const std::vector<std::string>& dirs)
{
    if (const char* dir = std::getenv(Config::CONFIG_PATH_ENV)) {
        for (const auto& name : names) {
            if (auto res = tryExpandPath(dir, name); !res.empty()) {
                return res.front();
            }
        }
    }

    for (const auto& dir : dirs) {
        for (const auto& name : names) {
            if (auto res = tryExpandPath(dir, name); !res.empty()) {
                return res.front();
            }
        }
    }
    return std::nullopt;
}

bool
is_ascii_whitespace_only(const std::u8string& str)
{
    return std::all_of(str.begin(), str.end(), [](char8_t ch) {
        return std::isspace(static_cast<unsigned char>(ch));
    });
}

std::u8string
substr_u8(const std::u8string& input, size_t start_char, size_t char_count)
{
    auto it = input.begin();
    for (size_t i = 0; i < start_char && it != input.end(); ++i) {
        if (it != input.end())
            ++it;
        while (it != input.end() && (*it & 0b11000000) == 0b10000000) {
            ++it;
        }
    }
    auto start_it = it;

    for (size_t i = 0; i < char_count && it != input.end(); ++i) {
        if (it != input.end())
            ++it;
        while (it != input.end() && (*it & 0b11000000) == 0b10000000) {
            ++it;
        }
    }

    return std::u8string(start_it, it);
}

std::u8string
read_file_to_u8_buffer(const fs::path& filename, int depth = 0)
{
    if (depth > 100) {
        throw std::runtime_error(
          "Aborting due to likely recursive include in config files");
    }

    std::u8string buffer;
    if (!fs::exists(filename) || !fs::is_regular_file(filename)) {
        spdlog::warn("File does not exist or is not a regular file.");
        return buffer;
    }

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        spdlog::warn("Failed to open file.");
        return buffer;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > 0) {
        buffer.resize(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(buffer.data()), size);

        size_t start_pos = 0;
        while ((start_pos = buffer.find(u8"include ", start_pos)) !=
               std::u8string::npos) {
            size_t prev_new_line_pos = buffer.rfind(u8"\n", start_pos);

            if (is_ascii_whitespace_only(substr_u8(
                  buffer, prev_new_line_pos, start_pos - prev_new_line_pos))) {
                size_t new_line_pos = buffer.find(u8"\n", start_pos);
                std::u8string included_filename;
                size_t new_line_offset = new_line_pos - start_pos - 8;
                if (new_line_pos == std::u8string::npos) {
                    new_line_offset = buffer.size() - start_pos - 8;
                }
                included_filename =
                  substr_u8(buffer, start_pos + 8, new_line_offset);
                fs::path parent_path = filename.parent_path();
                fs::path included_path = included_filename;
                if (included_filename.find(u8"/") == std::u8string::npos) {
                    included_path = parent_path / included_filename;
                }
                spdlog::info(
                  "Reading included file {}",
                  reinterpret_cast<const char*>(included_path.c_str()));

                std::u8string included_str =
                  read_file_to_u8_buffer(included_path, depth + 1);
                if (included_str.size() > 0) {
                    buffer.replace(
                      start_pos, new_line_offset + 8, included_str);
                    start_pos +=
                      included_str.length(); // Move past the replacement
                } else {
                    start_pos += 8;
                }
            } else {
                start_pos += 8;
            }
        }
    }

    return buffer;
}

void
Config::load(Gtk::Box* parent_box, const std::string& config)
{
    auto file =
      config.empty() ? findConfigPath({ "config.kdl", "config" }) : config;
    if (!file) {
        throw std::runtime_error("Missing required config files");
    }
    config_file_ = file.value();
    spdlog::info("Using configuration file {}", config_file_);
    std::u8string str = read_file_to_u8_buffer(config_file_);

    config_ = kdl::parse(str);

    width_ = DEFAULT_PANEL_WIDTH;
    margin_ = DEFAULT_PANEL_MARGIN;

    for (kdl::Node node : config_) {
        if (node.name() == u8"anchor") {
            for (auto anchorargs : node.args()) {
                if (anchorargs.as<std::u8string>() == u8"left") {
                    anchor_left_ = true;
                } else if (anchorargs.as<std::u8string>() == u8"right") {
                    anchor_right_ = true;
                } else if (anchorargs.as<std::u8string>() == u8"top") {
                    anchor_top_ = true;
                } else if (anchorargs.as<std::u8string>() == u8"bottom") {
                    anchor_bottom_ = true;
                }
            }
        }

        if (node.name() == u8"rows") {
            for (kdl::Node row_node : node.children()) {
                RowItem* cur_row_item =
                  new RowItem(parent_box, row_node, &rows_);
                rows_.push_back(cur_row_item);
            }
        }

        if (node.name() == u8"width") {
            width_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"margin") {
            margin_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"padding") {
            padding_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"spacing") {
            spacing_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"animation_duration") {
            animation_duration_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"rounded_corner") {
            rounded_corner_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"close_timeout") {
            close_timeout_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"close_on_escape") {
            close_on_escape_ = node.args()[0].as<int>();
        }

        if (node.name() == u8"title") {
            title_ = reinterpret_cast<const char*>(
              node.args()[0].as<std::u8string>().c_str());
        }

        if (node.name() == u8"use_layer") {
            auto config_layer = node.args()[0].as<std::u8string>();

            if (config_layer == u8"background") {
                use_layer_ = BACKGROUND;
            } else if (config_layer == u8"overlay") {
                use_layer_ = OVERLAY;
            } else if (config_layer == u8"top") {
                use_layer_ = TOP;
            } else if (config_layer == u8"bottom") {
                use_layer_ = BOTTOM;
            } else if (config_layer == u8"normal") {
                use_layer_ = NORMAL;
            }
        }
    }
}

size_t
Config::getRowSize()
{
    return rows_.size();
}

std::optional<RowItem*>
Config::getRow(const std::string& name)
{
    for (RowItem* row_item : rows_) {
        if (row_item->getName() == name) {
            return row_item;
        }
    }
    return std::nullopt;
}

std::optional<RowItem*>
Config::getRow(size_t index)
{
    if (index < rows_.size()) {
        return rows_[index];
    }
    return std::nullopt;
}

int
Config::getPanelMargin()
{
    return margin_;
}

int
Config::getPanelPadding()
{
    return padding_;
}

int
Config::getPanelWidth()
{
    return width_;
}

int
Config::getChildSpacing()
{
    return spacing_;
}

int
Config::getAnimationDuration()
{
    return animation_duration_;
}

int
Config::getCloseTimeout()
{
    return close_timeout_;
}

int
Config::getRoundedCorner()
{
    return rounded_corner_;
}

const std::string
Config::getListItemBoxHoverColor()
{
    return list_item_box_hover_color_;
}

bool
Config::getAnchorLeft()
{
    return anchor_left_;
}

bool
Config::getAnchorRight()
{
    return anchor_right_;
}

bool
Config::getAnchorTop()
{
    return anchor_top_;
}

bool
Config::getAnchorBottom()
{
    return anchor_bottom_;
}

int
Config::getCloseOnEscape()
{
    return close_on_escape_;
}

LayerType
Config::getUseLayer()
{
    return use_layer_;
}

std::string
Config::getTitle()
{
    return title_;
}

}