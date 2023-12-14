#include "Context.hpp"
#include "dnf2b/static/Constants.hpp"

#include <iostream>
#include <fstream>

namespace dnf2b {

Context::Context() {
    std::ifstream stream(Constants::DNF2B_ROOT / "config.local.json");
    if (!stream) {
        std::cerr << "Failed to find config.local.json" << std::endl;
        throw 255;
    }
    
    nlohmann::json custom;
    stream >> custom;

    config.update(custom, true);
}

// WTF is this even for? Most things cannot be health checked
void Context::checkHealth() {
    // TODO
}

int Context::getMaxAttempts() {
    return config.at("core").at("control").at("maxAttempts").get<int>();
}

}
