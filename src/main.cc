#include "helper/globals.hh"

#include <csignal>
#include <gtkmm.h>
#include <string>

#include "config/config.hh"
#include "lyra/lyra.hpp"
#include "spdlog/spdlog.h"
#include "window.hh"

using namespace medius;

helper::CliConfig cli_config;
config::Config main_config;

int
main(int argc, char* argv[])
{
    bool show_help = false;

    auto cli =
      lyra::help(show_help).description(
        "Highly customizable Wayland quick panel for Sway and Wlroots based "
        "compositors.") |

      lyra::cli() |

      lyra::opt(cli_config.config_opt,
                "config")["-c"]["--config"]("Config file path.") |

      lyra::opt(
        cli_config.log_level,
        "trace|debug|info|warning|error|critical|off")["-l"]["--log-level"](
        "The Log level.");

    auto result = cli.parse({ argc, argv });
    if (!result) {
        std::cerr << "Error in command line: " << result.message() << std::endl;
        exit(1);
    }

    if (show_help || !result) {
        std::cout
          << "Copyright (C) 2026 Rio Sanjaya oisanjaya@gmail.com\n"
             "This program comes with ABSOLUTELY NO WARRANTY\n"
             "This is free software, and you are welcome to redistribute it\n"
             "under certain conditions;\n\n";
        std::cout << cli << "\n";
        exit(1);
    }

    if (!cli_config.log_level.empty()) {
        spdlog::set_level(spdlog::level::from_str(cli_config.log_level));
    }

    auto app = Gtk::Application::create("id.my.oisanjaya.medius");
    return app->make_window_and_run<MainWindow>(0, nullptr);
}