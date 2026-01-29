#pragma once

#include "config/config.hh"
#include <string>

namespace helper {

using StaticOrDynamicCommandTuple = std::tuple<bool, std::string, int>;

struct CliConfig
{
    std::string log_level;
    std::string config_opt;
    std::string css_opt;
};

extern std::string latter_css_data;
extern bool disable_lost_focus_quit;

extern CliConfig cli_config;
extern config::Config main_config;
extern const std::string
executeCommand(const std::string& command, bool clean_new_line = true);
std::string
replaceString(std::string hay, std::string needle, std::string replacement);
StaticOrDynamicCommandTuple staticOrDynamicCommand(kdl::Node child);
std::string trim(const std::string& s);
bool isNumber(const std::string& str);

}