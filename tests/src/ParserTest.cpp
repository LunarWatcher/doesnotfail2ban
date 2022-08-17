#include "catch2/catch_test_macros.hpp"

#include "dnf2b/sources/FileParser.hpp"

TEST_CASE("Make sure trivial testing works", "[parser]") {
    dnf2b::FileParser p("journald");
    std::ifstream x("../etc/dnf2b/parsers/journald.json");
    REQUIRE(x.is_open());

    x >> p.config;
    INFO(p.config.dump());

    auto message = p.parse("Aug 17 22:17:44 sinon sshd[20094]: Failed password for invalid user admin from 123.45.67.89 port 57792 ssh2");
    REQUIRE(message);
    REQUIRE(message->host == "sinon");
    REQUIRE(message->process == "sshd");
    REQUIRE(message->message == "Failed password for invalid user admin from 123.45.67.89 port 57792 ssh2");
}
