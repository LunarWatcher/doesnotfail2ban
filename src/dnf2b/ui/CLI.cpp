#include "CLI.hpp"

#include <iostream>

#include "dnf2b/core/Daemon.hpp"

namespace dnf2b {

void CLI::parse(int argc, const char* argv[]) {
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    std::string command = args.size() == 0 ? "help" : args[0];

    if (args.size()) {
        args.erase(args.begin());
    }

    std::cout << "Command: " << command << "[ ";
    for (auto& arg : args) std::cout << arg << " ";
    std::cout << "]\n";

    Context c(args);

    if (command == "help") {
        std::cout << "Help goes here" << std::endl;
    } else if (command == "health") {
        c.checkHealth();
    } else if (command == "daemon") {
        Daemon{c}.run();
    } else if (command == "ban") {
        // TODO
    } else if (command == "unban") {

    }
}

}
