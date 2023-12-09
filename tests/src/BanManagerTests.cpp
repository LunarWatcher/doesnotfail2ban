#include "dnf2b/bouncers/NoopBouncer.hpp"
#include "dnf2b/data/BanManager.hpp"
#include "util/DBWrapper.hpp"
#include <bits/chrono.h>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <nlohmann/json.hpp>
#include <thread>

TEST_CASE("Verify whitelist logic", "[BanManager]")  {
    auto conf = 
#include <dnf2b/static/ConfDefault.hpp>

    conf["core"]["whitelist"] = {
        "192.168.0.1",
        "10.0.0.0/8",
        "12.34.56.78/16",
        "12.35.56.78"
    };
    conf["bouncers"]["noop"] = {
        {"stfu", ""},
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

TEST_CASE("Verify unban logic") {
    auto now = std::chrono::system_clock::now();
    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    tests::util::DBWrapper db;

    // TODO: find a better way to do this 
    auto conf = 
#include <dnf2b/static/ConfDefault.hpp>

    conf["bouncers"]["noop"] = {
        {"stfu", ""},
    };

    dnf2b::BanManager man(conf);

    dnf2b::IPInfo unban{
        .ip = "192.168.0.69",
        .banCount = 1,
        .currBans = {
            {"noop", {
                .banStarted = currTime,
                .banDuration = 10
            }}
        }
    };
    dnf2b::IPInfo stayBanned {
        .ip = "192.168.0.70",
        .banCount = 1,
        .currBans = {
            {"noop", {
                .banStarted = currTime,
                .banDuration = 1000
            }}
        }
    };

    dnf2b::NoopBouncer::_unban = [](const auto& ip, const auto& port) {
        if (ip != "192.168.0.69") {
            FAIL("Incorrect IP unbanned: " + ip);
        }
        if (port.has_value()) INFO(*port);
        REQUIRE_FALSE(port.has_value());
    };

    REQUIRE(unban.currBans["noop"].port.has_value() == false);
    db->updateIp(unban);
    db->updateIp(stayBanned);

    REQUIRE(db->getPendingUnbans().size() == 0);

    std::this_thread::sleep_until(now + std::chrono::seconds(10));
    REQUIRE(db->getPendingUnbans().size() == 1);
    REQUIRE(db->getPendingUnbans().at(0).port.has_value() == false);
    man.checkUnbansAndCleanup();

    REQUIRE(db->getPendingUnbans().size() == 0);
    REQUIRE(db->isBanned("192.168.0.70", "noop"));
    REQUIRE_FALSE(db->isBanned("192.168.0.69", "noop"));
}

TEST_CASE("Verify ban period") {

    auto now = std::chrono::system_clock::now();
    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    tests::util::DBWrapper db;

    // TODO: find a better way to do this 
    auto conf = 
#include <dnf2b/static/ConfDefault.hpp>

    conf["bouncers"]["noop"] = {
        {"stfu", ""},
    };

    conf["core"]["control"].merge_patch({
        {"banPeriod", "1w"},
        {"forgetAfter", "1w"}
    });

    dnf2b::BanManager man(conf);

    dnf2b::Watcher watcher(
        "demo",
        std::nullopt,
        69,
        0,
        {
            dnf2b::Filter("dummy-filter")
        },
        "noop"
    );

    std::vector<dnf2b::Message> messages = {
        {
            .entryDate = std::chrono::system_clock::now(),
            .message = "Foxes <3",
            .ip = "12.34.56.78"
        },
        {
            .entryDate = std::chrono::system_clock::now(),
            .message = "Foxxos <3",
            .ip = "87.65.43.21"
        },
    };

    auto res = watcher.process(messages);
    man.log(&watcher, res);

    REQUIRE(db->isBanned("12.34.56.78", "noop"));
    REQUIRE_FALSE(db->isBanned("87.65.43.21", "noop"));
    auto info = db->loadIp("12.34.56.78");
    REQUIRE(info.banCount == 1);
    REQUIRE(info.currFails.size() == 0);

    auto& currBans = info.currBans;
    REQUIRE(currBans.contains("noop"));
    REQUIRE(currBans.at("noop").banDuration == std::chrono::duration_cast<std::chrono::seconds>(std::chrono::weeks(1)).count());

    for (int i = 2; i < 10; ++i) {
        db->unban(info.ip, "noop");
        REQUIRE_FALSE(db->isBanned(info.ip, "noop"));

        res = watcher.process(messages);
        man.log(&watcher, res);
        REQUIRE(db->isBanned(info.ip, "noop"));

        info = db->loadIp(info.ip);
        REQUIRE(info.banCount == i);
    }
}
