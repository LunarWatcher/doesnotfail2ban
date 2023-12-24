#include "dnf2b/util/Clock.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

#include <dnf2b/data/MessageBuffer.hpp>

using namespace dnf2b;

TEST_CASE("Validate passthrough", "[MessageBuffer]") {
    auto buff = std::make_shared<MessageBuffer>(false);

    Filter f(std::string{"dummy-filter"});
    buff->load({
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.1", "69" },
        Message { Clock::now(), "a", "Foxes <3", "", "69" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
    });

    auto res = buff->forward(f);

    REQUIRE(res.size() == 2);
    // When the cache is off, the missing message should be discarded
    REQUIRE(res.at("192.168.0.1").size() == 1);
    REQUIRE(res.at("192.168.0.2").size() == 2);

    REQUIRE(buff->getOut().size() == 4);
    // Neither the buffer nor cache should have items
    REQUIRE(buff->getBuff().size() == 0);
    REQUIRE(buff->getHold().size() == 0);

    REQUIRE_NOTHROW(buff->done());
}

TEST_CASE("Validate buffer logic", "[MessageBuffer]") {
    auto buff = std::make_shared<MessageBuffer>(true);
    Filter f(std::string{"dummy-filter"});
    buff->load({
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.1", "69" },
        Message { Clock::now(), "a", "Foxes <3", "", "69" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
    });

    auto res = buff->forward(f);
    REQUIRE(res.size() == 2);
    REQUIRE(res.contains("192.168.0.1"));
    REQUIRE(res.at("192.168.0.1").size() == 2);
    REQUIRE(res.at("192.168.0.2").size() == 2);

    REQUIRE(buff->getOut().size() == 4);
    REQUIRE(buff->getHold().size() == 0);
    REQUIRE(buff->getBuff().size() == 2);
    REQUIRE(buff->getBuff().contains("69"));
    REQUIRE(buff->getBuff().contains("70"));

    buff->done();
    REQUIRE(buff->getOut().size() == 0);
    for (int i = 0; i < 7; ++i) {
        REQUIRE_NOTHROW(buff->done());
    }

    REQUIRE(buff->getOut().size() == 0);
    REQUIRE(buff->getHold().size() == 0);
    REQUIRE(buff->getBuff().size() == 0);
}

TEST_CASE("Validate hold", "[MessageBuffer]") {
    auto buff = std::make_shared<MessageBuffer>(true);

    Filter f(std::string{"dummy-filter"});
    buff->load({
        Message { Clock::now(), "a", "Foxes <3", "", "69" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
    });

    auto res = buff->forward(f);

    REQUIRE(res.size() == 1);
    REQUIRE(res.at("192.168.0.2").size() == 2);

    REQUIRE(buff->getOut().size() == 3);
    REQUIRE(buff->getBuff().size() == 1);
    REQUIRE(buff->getHold().size() == 1);

    REQUIRE_NOTHROW(buff->done());

    buff->load({
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.1", "69" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
        Message { Clock::now(), "a", "Foxes <3", "192.168.0.2", "70" },
    });
    res = buff->forward(f);

    REQUIRE(res.size() == 2);

    REQUIRE(res.at("192.168.0.1").size() == 2);
    REQUIRE(res.at("192.168.0.2").size() == 2);

    REQUIRE(buff->getOut().size() == 4);
    REQUIRE(buff->getBuff().size() == 2);
    REQUIRE(buff->getHold().size() == 0);
}

