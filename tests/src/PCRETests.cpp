#include "dnf2b/util/PCRE.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Patterns should compile", "[PCRE]") {
    REQUIRE_NOTHROW(dnf2b::Pattern("^I like trains$"));
    REQUIRE_NOTHROW(dnf2b::Pattern("^I'm (?<name>\\S+)"));
    REQUIRE_NOTHROW(dnf2b::Pattern("^I'm (?:\\S+)"));

    REQUIRE_THROWS(dnf2b::Pattern("["));
    REQUIRE_THROWS(dnf2b::Pattern("hel(()"));
}

TEST_CASE("Empty matches should not fuck the PCREMatcher", "[PCRE]") {
    dnf2b::Pattern p("^Hello, (\\S+)$");
    auto matcher = dnf2b::PCREMatcher(p, "Hello world!");
    REQUIRE_FALSE(matcher.next());
}

TEST_CASE("Patterns should match text", "[PCRE]") {
    dnf2b::Pattern p("^Hello, (\\S+)$");
    REQUIRE_FALSE(dnf2b::PCREMatcher(p, "Hello, you peasant").next());
    REQUIRE(dnf2b::PCREMatcher(p, "Hello, peasant").next());

    auto matcher = dnf2b::PCREMatcher(p, "Hello, spanishinquisition");
    REQUIRE(matcher.next());
    REQUIRE(matcher.getMatchGroups() == 2);

}

TEST_CASE("Multiple matches in the text should work", "[PCRE]") {
    dnf2b::Pattern p("Hello, ([^.]+)");
    std::string content = "Hello, A. Hello, B. Hello, C.";
    auto matcher = dnf2b::PCREMatcher(p, content);
    std::vector<std::string> matchGroupResults = {
        "A", "B", "C"
    };
    std::vector<std::string> fullMatches = {
        "Hello, A", "Hello, B", "Hello, C"
    };
    SECTION("There should be three matches with two groups each") {
        size_t count = 0;
        bool match;
        do {
            match = matcher.next();
            if (match) {
                REQUIRE(matcher.getMatchGroups() == 2);
                REQUIRE(matcher.get(0) == fullMatches[count]);
                REQUIRE(matcher.get(1) == matchGroupResults[count]);
                REQUIRE_THROWS(matcher.get(2));

                ++count;
            }
        } while (match);
        REQUIRE(count == 3);
    }
}

TEST_CASE("Named matches should work", "[PCRE]") {
    dnf2b::Pattern p("Caught (?<IP>\\S+) doing evil stuff");
    std::string message = "Caught 192.168.69.420 doing evil stuff";

    dnf2b::PCREMatcher m(p, message);
    REQUIRE(m.next());
    REQUIRE(m.get("IP") == "192.168.69.420");
    REQUIRE_THROWS(m.get("joemama"));
    REQUIRE_THROWS(m.get(56201));
    REQUIRE_NOTHROW(m.get(0));
    REQUIRE_NOTHROW(m.get(1));
    REQUIRE_THROWS(m.get(2));
    REQUIRE_FALSE(m.next());

}
