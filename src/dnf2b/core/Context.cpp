#include "Context.hpp"

#include <iostream>
#include <fstream>

namespace dnf2b {

// TODO: figure out how yaml-cpp error handles LoadFile()
Context::Context(const std::vector<std::string>& arguments) :  arguments(arguments) {
    std::ifstream stream("/etc/dnf2b/config.local.json");
    if (!stream) {
        std::cerr << "Failed to find config.local.json" << std::endl;
        // TODO: make a new exception type, catch in main(), return 255 to avoid continued reboots
        throw std::runtime_error("Config load failed");
    }
    
    nlohmann::json custom;
    stream >> custom;

    config.update(custom, true);

}

void Context::checkHealth() {
    // TODO
}

}
