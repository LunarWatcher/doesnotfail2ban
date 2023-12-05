#include "dnf2b/data/BanManager.hpp"
#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

TEST_CASE("Verify whitelist logic", "[BanManager]")  {
    auto conf = 
#include <dnf2b/static/ConfDefault.hpp>

    conf["core"]["whitelist"] = {
        "192.168.0.1",
        "10.0.0.0/8",
        "12.34.56.78/16",
        "12.35.56.78"
    };

    dnf2b::BanManager man(conf);

    SECTION("Validate direct matches") {
        REQUIRE(!man.isWhitelisted("192.168.0.0"));
        REQUIRE(man.isWhitelisted("192.168.0.1"));
        REQUIRE(man.isWhitelisted("12.35.56.78"));
        REQUIRE(!man.isWhitelisted("12.35.57.78"));
        REQUIRE(!man.isWhitelisted("12.35.56.79"));
        REQUIRE(!man.isWhitelisted("11.36.56.79"));

        REQUIRE_THROWS([&]() {
            man.isWhitelisted("192.168.0.256");
        }());
        REQUIRE_THROWS([&]() {
            man.isWhitelisted("512.168.0.69");
        }());
    }

    SECTION("Validate CIDR ranges") {
        REQUIRE(man.isWhitelisted("10.11.12.13"));
        REQUIRE(man.isWhitelisted("10.0.0.1"));
        REQUIRE(man.isWhitelisted("10.255.255.255"));
        REQUIRE(man.isWhitelisted("10.0.12.255"));
        REQUIRE(man.isWhitelisted("10.11.255.13"));
        REQUIRE(man.isWhitelisted("10.255.12.13"));
        REQUIRE(!man.isWhitelisted("11.0.0.0"));

        REQUIRE(man.isWhitelisted("12.34.0.0"));
        REQUIRE(man.isWhitelisted("12.34.56.78"));
        REQUIRE(!man.isWhitelisted("12.33.255.255"));
    }
}
