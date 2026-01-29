#include "helper/globals.hh"
#include "kdlpp.h"
#include <spdlog/spdlog.h>
#include <string>

namespace helper {

std::string latter_css_data = "";
bool disable_lost_focus_quit = false;

CliConfig cli_config;
config::Config main_config;

std::string
trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool
isNumber(const std::string& str)
{
    std::string s = trim(str);
    if (s.empty())
        return false;

    bool decimalPointSeen = false;
    bool digitSeen = false;
    size_t start = 0;

    if (s[0] == '+' || s[0] == '-') {
        start = 1;
        if (s.size() == 1)
            return false; // Only sign, no digits
    }

    for (size_t i = start; i < s.size(); ++i) {
        if (std::isdigit(s[i])) {
            digitSeen = true;
        } else if (s[i] == '.') {
            if (decimalPointSeen)
                return false; // More than one decimal point
            decimalPointSeen = true;
        } else {
            return false; // Invalid character
        }
    }

    return digitSeen; // Must have at least one digit
}

const std::string
executeCommand(const std::string& command, bool clean_new_line)
{
    char buffer[128];
    std::string result;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int status = pclose(pipe);
    if (status == -1) {
        throw std::runtime_error("Error closing the pipe!");
    }

    if (clean_new_line) {
        result.erase(std::remove(result.begin(), result.end(), '\n'),
                     result.end());
    }

    spdlog::debug("Run command:\n{}\n\nResult:\n{}//", command, result);

    return result;
}

std::string
replaceString(std::string hay, std::string needle, std::string replacement)
{
    auto ret_str = hay;
    size_t pos = 0;

    while ((pos = ret_str.find(needle, pos)) != std::string::npos) {
        ret_str.replace(pos, needle.length(), replacement);
        pos += replacement.length();
    }

    return ret_str;
}

StaticOrDynamicCommandTuple
staticOrDynamicCommand(kdl::Node child)
{
    StaticOrDynamicCommandTuple retval(false, "", -1);

    if (child.children().size() > 0) {
        std::get<0>(retval) = true;
        for (auto grandclhild : child.children()) {
            if (grandclhild.name() == u8"interval") {
                std::get<2>(retval) = grandclhild.args()[0].as<int>();
            } else if (grandclhild.name() == u8"command") {
                std::get<1>(retval) = reinterpret_cast<const char*>(
                  grandclhild.args()[0].as<std::u8string>().c_str());
            }
        }
    } else {
        std::get<1>(retval) = reinterpret_cast<const char*>(
          child.args()[0].as<std::u8string>().c_str());
    }

    return retval;
}

}