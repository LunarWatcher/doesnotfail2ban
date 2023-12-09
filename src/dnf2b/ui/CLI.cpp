#include "CLI.hpp"

#include <iostream>

#include "dnf2b/core/Daemon.hpp"
#include "spdlog/spdlog.h"
#include "stc/FileLock.hpp"
#include "fmt/format.h"

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
            << format("daemon", "Starts the dnf2b daemon");
        std::cout << "Manual management:" << std::endl;
        std::cout
            << format("ban", "Manually ban one or more IPs")
            << format("unban", "Manually unban one or more IPs");

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
            spdlog::error("Failed to acquire daemon lock. Is dnf2b running already?");
            return -2;
        } 
        
        Daemon{c}.run();
    } else if (command == "ban") {
        // TODO
    } else if (command == "unban") {

    }

    return 0;
}

}
