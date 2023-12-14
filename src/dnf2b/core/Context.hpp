#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "dnf2b/bouncers/Bouncer.hpp"
#include "dnf2b/sources/Parser.hpp"

#include <nlohmann/json.hpp>

namespace dnf2b {

class Context {
private:
    nlohmann::json config = 
#include "dnf2b/static/ConfDefault.hpp"


    std::map<std::string, std::shared_ptr<Parser>> parsers;
public:
    Context();

    const nlohmann::json& getConfig() { return config; }
    const nlohmann::json& getConfig() const { return config; }
    int getMaxAttempts();

    /**
     * Entry point for this class' health check, as well as all classes contained by it.
     * The Context class contains and manages all the core classes.
     */
    void checkHealth();

};

}
