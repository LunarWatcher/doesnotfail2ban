#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "dnf2b/bouncers/Bouncer.hpp"

#include <yaml-cpp/yaml.h>

namespace dnf2b {

class Context {
private:
    YAML::Node config;


public:

    // Top-level command arguments
    const std::vector<std::string> arguments;

    Context(const std::vector<std::string>& arguments);

};

}
