#include "dnf2b/json/Config.hpp"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Validate feature loading") {
    const std::string in = R"(
{
    "_comment": "This is a dummy field to work around the lack of comments in JSON. Both the watcher and bouncer fields will eventually be cleared. Do not rely on the defaults to never change",
    "core": {
        "control": {
            "maxAttempts": 2,
            "banPeriod": 1,
            "banIncrement": 3,
            "forgetAfter": 1
        },
        "stats": {
            "enabled": false,
            "banStats": false,
            "originStats": false,
            "credentialStats": false
        },
        "whitelist": [
            "1.1.1.1"
        ]
    },
    "watchers": [
        {
            "id": "sshd",
            "process": "sshd",
            "enabled": true,
            "parser": "journald",
            "filters": ["sshd-bruteforce"],
            "banaction": "iptables"
        }
    ],
    "bouncers": {
        "iptables": {
            "strategy": "DROP"
        }
    }
}
    )";

    dnf2b::ConfigRoot cfg = nlohmann::json::parse(in);

    REQUIRE(cfg.core.whitelist.size() == 1);
    REQUIRE(cfg.core.whitelist.at(0) == "1.1.1.1");
    REQUIRE(cfg.core.control.banIncrement == 3);
    REQUIRE(cfg.core.control.forgetAfter == 60 * 60 * 24);
    REQUIRE(cfg.core.control.banPeriod == 60 * 60 * 24);
    REQUIRE(cfg.core.control.maxAttempts == 2);

    REQUIRE_FALSE(cfg.core.stats.banStats);
    REQUIRE_FALSE(cfg.core.stats.credentialStats);
    REQUIRE_FALSE(cfg.core.stats.originStats);
    REQUIRE_FALSE(cfg.core.stats.enabled);

    REQUIRE(cfg.watchers.size() == 1);
    REQUIRE(cfg.bouncers.size() == 1);

    REQUIRE(cfg.watchers.at(0).at("id") == "sshd");
    REQUIRE(cfg.bouncers.contains("iptables"));

}
