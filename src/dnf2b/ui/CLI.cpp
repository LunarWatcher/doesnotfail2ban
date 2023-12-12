#include "CLI.hpp"

#include <iostream>

#include "dnf2b/core/Daemon.hpp"
#include "spdlog/spdlog.h"
#include "stc/FileLock.hpp"
#include "fmt/format.h"

#include <thread>
#include <unistd.h>

namespace dnf2b {

int CLI::parse(int argc, const char* argv[]) {
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    std::string command = args.size() == 0 ? "help" : args[0];

    if (args.size()) {
        args.erase(args.begin());
    }

    //std::cout << "Command: " << command << "[ ";
    //for (auto& arg : args) std::cout << arg << " ";
    //std::cout << "]\n";


    auto format = [](const std::string& command, const std::string& description) {
        return fmt::format("\t{:<16} {}\n", command, description);
    };

    if (command == "help") {
        std::cout << "Commands:" << std::endl;
        std::cout << "General:" << std::endl;
        std::cout 
            << format("help", "Shows this helpful message")
            << format("health", "Runs a health check on the server")
            << format("daemon", "Starts the dnf2b daemon")
            << format("delete-lockfile", "Deletes the daemon lockfile. DO NOT RUN unless there's no daemon already running. This can and will break stuff.");
        std::cout << "Manual management:" << std::endl;
        std::cout
            << format("ban", "Manually ban one or more IPs")
            << format("unban", "Manually unban one or more IPs");

        return 0;
    } else if (command == "version") {
        std::cout << "doesnotfail2ban version " << DNF2B_VERSION << std::endl;
        return 0;
    }

    if (getuid() != 0) {
        std::cerr << "Must be root to run other non-help commands." << std::endl;
        return -1;
    }

    Context c(args);

    if (command == "health") {
        c.checkHealth();
    } else if (command == "daemon") {
        std::shared_ptr<stc::FileLock> lock;
        try {
            lock = std::make_shared<stc::FileLock>("/var/run/lock/dnf2b.daemon.lock");
        } catch (stc::FileLock::Errors e) {
            spdlog::error("Failed to acquire daemon lock. Is dnf2b running already? If this is a mistake, run dnf2b delete-lockfile");
            return -2;
        } 
        
        Daemon{c}.run();
    } else if (command == "ban") {
        // TODO
    } else if (command == "unban") {

    } else if (command == "delete-lockfile") {
        std::cout << "WARNING: Running this command while the daemon is running can and will result in bad stuff happening. "
            << "Before running, make sure dnf2b is, in fact, not running. You can do this by running `ps aux | grep dnf2b`"
            << "\n"
            << "If you're in doubt, press Ctrl-C immediately. Otherwise, the lockfile will be deleted in 15 seconds."
            << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(15));
        std::filesystem::remove("/var/run/lock/dnf2b.daemon.lock");
    } else {
        std::cerr << "Invalid command: " << command << std::endl;
        return -1;
    }

    return 0;
}

}
