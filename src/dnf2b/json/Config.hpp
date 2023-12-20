#pragma once

#include "nlohmann/detail/macro_scope.hpp"
#include "nlohmann/json.hpp"

namespace dnf2b {

struct ControlConfig {
    long long maxAttempts;
    long long banIncrement;
    long long banPeriod;
    long long forgetAfter;

    ControlConfig() 
        : maxAttempts(3),
          banIncrement(2),
          // Both the next two variables default to two weeks
          banPeriod(60 * 60 * 24 * 7 * 2),
          forgetAfter(60 * 60 * 24 * 7 * 2)
    {}

};

extern void from_json(const nlohmann::json& j, ControlConfig& o);
extern void to_json(nlohmann::json& j, const ControlConfig& o);

struct StatConfig {
    bool enabled, banStats, originStats, credentialStats;
    StatConfig() : enabled(true), banStats(true), originStats(true), credentialStats(true) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        StatConfig,
        enabled,
        banStats,
        originStats,
        credentialStats
    );
};


struct CoreConfig {
    ControlConfig control;
    StatConfig stats;

    std::vector<std::string> whitelist;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        CoreConfig,
        control,
        stats
    );
};

struct ConfigRoot {
    CoreConfig core;
    // These are difficult to parse due to the many different types
    // involved.
    // They're kept as json objects, and it's up to the watchers and
    // bouncers to parse their own type correctly.
    nlohmann::json watchers;
    nlohmann::json bouncers;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        ConfigRoot,
        core,
        watchers,
        bouncers
    );
};

}
