#include "dnf2b/data/BanDB.hpp"
#include "dnf2b/data/BanManager.hpp"
#include "dnf2b/data/Structs.hpp"
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

        std::this_thread::sleep_for(std::chrono::seconds(10));

        REQUIRE(db->getPendingUnbans().size() == 1);
        db->unban(info.ip, "noop");
        REQUIRE_FALSE(db->isBanned(info.ip, "noop"));

    }

}

TEST_CASE("BanDB should integrate with BanManager", "[Database][BanManager]") {
    tests::util::DBWrapper db;
    auto conf = 
#include <dnf2b/static/ConfDefault.hpp>

    conf["bouncers"]["noop"] = {
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
