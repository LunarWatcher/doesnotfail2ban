#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "dnf2b/bouncers/Bouncer.hpp"

#include <nlohmann/json.hpp>

namespace dnf2b {

class Context {
private:
    nlohmann::json config = 
#include "dnf2b/static/ConfDefault.hpp"
        ;


public:

    // Top-level command arguments
    const std::vector<std::string> arguments;

    Context(const std::vector<std::string>& arguments);

    void checkHealth();

};

}
