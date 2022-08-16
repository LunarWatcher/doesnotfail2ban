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

public:

    // Top-level command arguments
    const std::vector<std::string> arguments;

    Context(const std::vector<std::string>& arguments);

    /**
     * Hooks up data related to the current configuration of dnf2b
     */
    void start();

    void poll();

    const nlohmann::json& getConfig() { return config; }

    /**
     * Entry point for this class' health check, as well as all classes contained by it.
     * The Context class contains and manages all the core classes.
     */
    void checkHealth();

};

}
