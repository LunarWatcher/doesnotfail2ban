#include "catch2/catch_test_macros.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/sources/Parser.hpp"
#include "dnf2b/watcher/Watcher.hpp"
#include <chrono>

TEST_CASE("Validate processing logic", "[Watcher]") {
    dnf2b::Watcher watcher(
        "demo",
        std::nullopt,
        69,
        0,
        {
            dnf2b::Filter((std::string) "dummy-filter")
        },
        ""
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

    auto res = watcher.process(messages, std::make_shared<dnf2b::MessageBuffer>(false));
    REQUIRE(res.size() == 1);
    REQUIRE(res.contains("12.34.56.78"));
    REQUIRE(res.at("12.34.56.78").size() == 1);
}
