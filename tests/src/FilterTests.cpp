#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/static/Constants.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

TEST_CASE("All the stock filters should compile", "[Filters]") {
    for (auto& file : std::filesystem::directory_iterator(dnf2b::Constants::DNF2B_ROOT / "filters")) {
        if (file.is_regular_file()) {
            auto path = file.path();
            if (path.extension() == ".json") {
                auto name = path
                    .replace_extension() // Drop .json extension
                    .filename() // Drop path
                    .string();
                if (name.starts_with("dummy")) {
                    continue;
                }
                INFO(name);
                REQUIRE_NOTHROW(dnf2b::Filter(name));
            }
        }
    }
}

TEST_CASE("The filter class should load @scope/filter-name filters", "[Filters]") {
    std::filesystem::create_directories(dnf2b::Constants::DNF2B_ROOT / "custom/filtertests-1/filters");
    {
        std::ofstream f(dnf2b::Constants::DNF2B_ROOT / "custom/filtertests-1/filters/dummy-filter.json");
        REQUIRE(f);
        auto testJson = nlohmann::json {
            {"patterns", {
                "a",
                "potato",
                "17",
                "hut"
            }}
        };
        f << testJson;
    }

    REQUIRE_NOTHROW(dnf2b::Filter("@filtertests-1/dummy-filter"));

    auto dummyA = dnf2b::Filter("dummy-filter");
    auto dummyB = dnf2b::Filter("@filtertests-1/dummy-filter");

    const auto& p1 = dummyA.getPatterns();
    const auto& p2 = dummyB.getPatterns();
    REQUIRE(p1.size() != p2.size());
}

TEST_CASE("Validate passthrough behaviour for non-scoped filters", "[Filters]") {
    std::filesystem::create_directories(dnf2b::Constants::DNF2B_ROOT / "custom/filtertests-2/filters");
    {
        std::ofstream f(dnf2b::Constants::DNF2B_ROOT / "custom/filtertests-2/filters/i-like-trains.json");
        REQUIRE(f);
        auto testJson = nlohmann::json {
            {"patterns", {
                "a",
                "potato",
                "17",
                "hut"
            }}
        };
        f << testJson;
    }
    REQUIRE_NOTHROW(dnf2b::Filter("@filtertests-2/i-like-trains"));
    REQUIRE_NOTHROW(dnf2b::Filter("i-like-trains"));
}

TEST_CASE("Error handling for non-existent filters", "[Filters]") {
    REQUIRE_THROWS(dnf2b::Filter("this-will-never-exist-aaaaaaaaaaaaaaaa"));
    REQUIRE_THROWS(dnf2b::Filter("@50-fucking-potatoes/this-will-never-exist-aaaaaaaaaaaaaaaa"));
}
