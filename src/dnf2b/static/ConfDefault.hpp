// Wtf?, why did I do this?
nlohmann::json {
    {"core", {
        {"control", {
            {"maxAttempts", 3},
            {"searchPeriod", "7d"},
            {"banPeriod", "1w"},
        }},
        {"stats", {
            {"enabled", true},
            {"banStats", true},
            {"originStats", true},
            {"credentialStats", true},
            {"blockStats", true}
        }}
    }},
    // We don't enable watchers and bouncers by default here; far too annoying
    // to deal with.
    {"watchers", {}},
    {"bouncers", {}}
};
