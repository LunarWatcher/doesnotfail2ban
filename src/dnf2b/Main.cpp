#include <fstream>
#include <iostream>
#include <stdexcept>

#include <CLI/CLI.hpp>
#include "nlohmann/json.hpp"
#include "dnf2b/json/Config.hpp"
#include "dnf2b/static/Constants.hpp"
#include "spdlog/cfg/helpers.h"
#include "spdlog/spdlog.h"
#include "stc/Environment.hpp"
#include "dnf2b/core/Daemon.hpp"
#include <dnf2b/cli/FilterWizard.hpp>
#include <dnf2b/cli/Tester.hpp>

#ifdef DNF2B_DEBUG_PATH
#warning "Building with a relative path to dnf2b's config dir. THIS WILL NOT WORK IN PRODUCTION!"
#warning "Disable DNF2B_DEBUG_PATH for production use"
std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "../etc/dnf2b";
#else
std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "/etc/dnf2b";
#endif

int main(int argc, const char* argv[]) {
    std::string envVal = stc::getEnv("SPDLOG_LEVEL", "info");
    spdlog::cfg::helpers::load_levels(envVal);

    auto checkRoot = []() {
        if (getuid() != 0) {
            std::cerr << "Must be root to run this command." << std::endl;
            throw -1;
        }
    };

    try {

        CLI::App app{"Intrusion detection and blocking system"};
        app.set_help_all_flag("--help-all", "Expand all help");

        app.add_subcommand("version", "Shows the current version")
            ->group("General")
            ->callback([]() {
                std::cout << "doesnotfail2ban version " << DNF2B_VERSION << std::endl;
            });
        app.add_subcommand("daemon", "Runs dnf2b. Note that this is a blocking command, and should not be used to start dnf2b in the background. Use\n\tsudo systemctl start dnf2b\ninstead")
            ->group("General")
            ->callback([&]() -> void {

                std::shared_ptr<stc::FileLock> lock;
                try {
                    lock = std::make_shared<stc::FileLock>("/var/run/lock/dnf2b.daemon.lock");
                } catch (stc::FileLock::Errors e) {
                    spdlog::error("Failed to acquire daemon lock. Is dnf2b running already? If this is a mistake, run dnf2b delete-lockfile");
                } 
                
                dnf2b::ConfigRoot c;
                {
                    std::ifstream f("/etc/dnf2b/config.local.json");
                    if (!f) {
                        spdlog::error("Please create config.local.json before starting the client. See the docs for more info,"
                                      "and /etc/dnf2b/config.json for a template.");
                        throw std::runtime_error("Config load error");
                    }

                    nlohmann::json cfg;
                    f >> cfg;
                    c = cfg;
                }

                dnf2b::Daemon{c}.run();
            })
            ->parse_complete_callback(checkRoot);

        {
            std::string ip;
            auto command = app.add_subcommand("ban", "Ban a specified IP")
                ->group("Admin")
                ->callback([&]() {
                    std::cout << "TODO: " << ip << std::endl;
                })
                ->parse_complete_callback(checkRoot);
            command->add_option("IP", ip, "The IP to unban")
                ->required();
        }
        {
            std::string ip;
            auto command = app.add_subcommand("unban", "Unban a specified IP")
                ->group("Admin")
                ->callback([&]() {
                    std::cout << "TODO" << std::endl;
                })
                ->parse_complete_callback(checkRoot);
            command->add_option("IP", ip, "The IP to unban")
                ->required();
        }
        app.add_subcommand("delete-lockfile", "Deletes the lockfile if it exists. DO NOT USE unless you're absolutely sure no other daemon is running")
            ->group("Meta")
            ->callback([&]() {

                std::cout << "WARNING: Running this command while the daemon is running can and will result in bad stuff happening. "
                    << "Before running, make sure dnf2b is, in fact, not running. You can do this by running `ps aux | grep dnf2b`"
                    << "\n"
                    << "If you're in doubt, press Ctrl-C immediately. Otherwise, the lockfile will be deleted in 15 seconds."
                    << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(15));
                std::filesystem::remove("/var/run/lock/dnf2b.daemon.lock");
            })
            ->parse_complete_callback(checkRoot);
        { 
            bool caseInsensitive;

            auto command = app.add_subcommand("filter-wizard", "Utility for creating filters")
                ->group("Utility")
                ->callback([&]() {
                    dnf2b::CLI::filterWizard(caseInsensitive);
                });
            command->add_option("insensitive", caseInsensitive, "Whether or not the filters should be case-insensitive")
                ->default_val(true);
        }

        {
            std::string testString;
            std::string parser;
            auto command = app.add_subcommand("test", "Utility for testing filters and parsers at once. The parser must "
                                              "be supplied, but the resulting message is tested against all filters in "
                                              "existence")
                ->group("Utility")
                ->callback([&]() {
                    dnf2b::CLI::testLineMatches(testString, parser);
                });
            command->add_option("--parser", parser, "The parser to use")
                ->required(true);
            command->add_option("test_string", testString, "The full message string to test against.")
                ->required(true);
        }


        CLI11_PARSE(app, argc, argv);

    } catch (const int& x) {
        // I'm abusing int throwing as an overcomplicated way to return the value,
        // because why not? It works, and it's the only way to do this in a system where
        // whether the errors warrant a restart is up to the specific error.
        //
        // Future me here: What the actual fuck was past me thinking?
        return x;
    } catch (const std::runtime_error& e) {
        spdlog::error("Runtime exception caught: {}", e.what());
        return 255;
    }
    return 0;
}
