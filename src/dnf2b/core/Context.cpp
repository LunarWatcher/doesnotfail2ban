#include "Context.hpp"

#include <iostream>
#include <fstream>

namespace dnf2b {

// TODO: figure out how yaml-cpp error handles LoadFile()
Context::Context(const std::vector<std::string>& arguments) :  arguments(arguments) {
    std::ifstream stream("/etc/dnf2b/config.local.json");
    if (!stream) {
        std::cerr << "Failed to find config.local.json" << std::endl;
        throw 255;
    }
    
    nlohmann::json custom;
    stream >> custom;

    config.update(custom, true);
}

void Context::checkHealth() {
    // TODO
}

}
