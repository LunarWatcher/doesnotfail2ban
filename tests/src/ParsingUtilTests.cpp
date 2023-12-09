#include "dnf2b/util/Parsing.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Date parsing", "[Parsing][Date]") {
    SECTION("Parsing negative dates should return -1") {
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds("-1") == -1);
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds(-1) == -1);
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds(-1549876ll) == -1);
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds("-69w") == -1);
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds("-69m") == -1);
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds("-69d") == -1);
        REQUIRE(dnf2b::Parsing::parseConfigToSeconds("-69invalid but that's fine") == -1);
    }
    SECTION("Parsing invalid date formats should throw") {
        REQUIRE_THROWS([]() {
            dnf2b::Parsing::parseConfigToSeconds("69i");
        }());
        REQUIRE_THROWS([]() {
            dnf2b::Parsing::parseConfigToSeconds("69s");
        }());
        REQUIRE_THROWS([]() {
            dnf2b::Parsing::parseConfigToSeconds("69å");
        }());
    }
    SECTION("Parsing unquantified dates should parse them as days") {
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds("12")
            ==
            12 * 24 * 60 * 60
        );
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds(12)
            ==
            12 * 24 * 60 * 60
        );

        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds("1")
            ==
            1 * 24 * 60 * 60
        );
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds(1)
            ==
            1 * 24 * 60 * 60
        );
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds("1d")
            ==
            1 * 24 * 60 * 60
        );
    }
    SECTION("Parsing should be correct") {
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds("10d")
            ==
            10 * 24 * 60 * 60
        );
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds("2w")
            ==
            2 * 7 * 24 * 60 * 60
        );
        REQUIRE(
            dnf2b::Parsing::parseConfigToSeconds("1m")
            ==
            // 1 month in std::chrono is, in fact, not a month.
            // It's (basically) the number of seconds in a year divided by 12,
            // because fuck you
            //
            // Doesn't make a difference for dnf2b in practice, though. 
            // I doubt anyone using systems like these rely on the unbans to happen
            // _exactly_ when the ban period ends.
            2629746

        );
    }
}
