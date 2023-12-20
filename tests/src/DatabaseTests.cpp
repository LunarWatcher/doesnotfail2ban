#include "dnf2b/data/BanDB.hpp"
#include "dnf2b/data/BanManager.hpp"
#include "dnf2b/data/Structs.hpp"
#include "dnf2b/json/Config.hpp"
#include "dnf2b/static/Constants.hpp"
#include "dnf2b/watcher/Watcher.hpp"
#include "util/DBWrapper.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <filesystem>
#include <thread>


TEST_CASE("It should handle ban logic", "[Database]") {
    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    tests::util::DBWrapper db;

    dnf2b::IPInfo info{
        .ip = "192.168.0.69",
        .currFails = {
            currTime - 300,
            currTime - 600,
            currTime - 900
        },
        .banCount = 0
    };

    REQUIRE_FALSE(db->isBanned(info.ip, "noop"));

    db->updateIp(info);

    REQUIRE_FALSE(db->isBanned(info.ip, "noop"));
    
    SECTION("The saved entry should match the entry in the database"){
        dnf2b::IPInfo loaded = db->loadIp(info.ip);
        REQUIRE(loaded.currBans.size() == info.currBans.size());
        REQUIRE_THAT(info.currFails, Catch::Matchers::UnorderedEquals(loaded.currFails));
    }

    SECTION("Forgiving old fails should discard sufficiently old fails") {
        db->forgiveFails(450);
        auto _info = db->loadIp(info.ip);
        REQUIRE(_info.currFails.size() == 1);
    }
    SECTION("Forgiving old fails should discard none when none are old enough") {
        db->forgiveFails(1450);
        auto _info = db->loadIp(info.ip);
        REQUIRE(_info.currFails.size() == 3);
    }

    SECTION("Banning should work") {
        info.currFails.clear();
        info.currBans["noop"] = {
            .banStarted = currTime,
            .banDuration = 10
        };
        db->updateIp(info);
        REQUIRE(db->isBanned(info.ip, "noop"));
        REQUIRE_FALSE(db->isBanned(info.ip, "iptables"));
        REQUIRE(db->getPendingUnbans().size() == 0);

        auto loaded = db->loadIp(info.ip);
        REQUIRE(loaded.currBans.contains("noop"));
        REQUIRE_FALSE(loaded.currBans.at("noop").port.has_value());

        std::this_thread::sleep_for(std::chrono::seconds(10));

        REQUIRE(db->getPendingUnbans().size() == 1);
        db->unban(info.ip, "noop");
        REQUIRE_FALSE(db->isBanned(info.ip, "noop"));

    }

    SECTION("Ports should be registered") {
        info.currFails.clear();
        info.currBans["noop"] = {
            .banStarted = currTime,
            .banDuration = 10,
            .port = 69
        };

        db->updateIp(info);

        auto loaded = db->loadIp(info.ip);
        REQUIRE(loaded.currBans.contains("noop"));
        REQUIRE(loaded.currBans.at("noop").port.has_value());
        REQUIRE(loaded.currBans.at("noop").port.value() == 69);

    }

}

TEST_CASE("Re-ban core logic should work") {
    auto now = std::chrono::system_clock::now();
    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    tests::util::DBWrapper db;

    dnf2b::IPInfo stayingBanned {
        .ip = "192.168.0.69",
        .banCount = 1,
        .currBans = {
            {"noop", {
                .banStarted = currTime,
                .banDuration = 6969
            }}
        }
    };

    dnf2b::IPInfo awaitingUnban {
        .ip = "192.168.0.70",
        .banCount = 1,
        .currBans = {
            {"noop", {
                .banStarted = currTime - 69,
                .banDuration = 42
            }}
        }
    };

    db->updateIp(stayingBanned);
    db->updateIp(awaitingUnban);

    auto unbanQueue = db->getPendingUnbans();
    auto rebanQueue = db->getBannedMinusPendingUnbans();

    REQUIRE(unbanQueue.size() == 1);
    REQUIRE(rebanQueue.size() == 1);
    REQUIRE(unbanQueue.at(0).ip == "192.168.0.70");
    REQUIRE(rebanQueue.at(0).ip == "192.168.0.69");

}

TEST_CASE("BanDB should integrate with BanManager", "[Database][BanManager]") {
    tests::util::DBWrapper db;
    dnf2b::ConfigRoot conf;

    conf.bouncers["noop"] = {
        {"stfu", ""},
    };
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
    
    auto watcherRes = watcher.process({
        {
            .entryDate = std::chrono::system_clock::now(),
            .message = "Foxes <3",
            .ip = "12.34.56.78"
        },
        {
            .entryDate = std::chrono::system_clock::now(),
            .message = "Never gonna give you up",
            .ip = "12.34.56.78"
        },
        {
            .entryDate = std::chrono::system_clock::now(),
            .message = "Foxxos <3",
            .ip = "87.65.43.21"
        },
    });

    REQUIRE(watcherRes.size() == 1);
    REQUIRE(watcherRes.at("12.34.56.78") == 2);

    REQUIRE_FALSE(db->isBanned("12.34.56.78", "noop"));
    man.log(&watcher, watcherRes);
    REQUIRE(db->isBanned("12.34.56.78", "noop"));
}
