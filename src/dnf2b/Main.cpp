#include <iostream>
#include <stdexcept>

#include <CLI/CLI.hpp>
#include "dnf2b/core/Context.hpp"
#include "dnf2b/static/Constants.hpp"
#include "spdlog/cfg/helpers.h"
#include "spdlog/spdlog.h"
#include "stc/Environment.hpp"
#include "dnf2b/core/Daemon.hpp"

std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "/etc/dnf2b";

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

        CLI::App app{"Log monitoring and IP ban system"};
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
                
                // TODO: This use of Context is stupid. Probably need to rethink the entire class
                dnf2b::Context c;
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
