#include "catch2/catch_test_macros.hpp"

#include "dnf2b/sources/FileParser.hpp"
#include <iostream>

#include <time.h>

TEST_CASE("Make sure trivial testing works", "[parser]") {
    dnf2b::FileParser p("journald");
    std::ifstream x("../etc/dnf2b/parsers/journald.json");
    REQUIRE(x.is_open());

    x >> p.config;
    INFO(p.config.dump());

    auto message = p.parse("Aug 17 22:17:44 sinon sshd[20094]: Failed password for invalid user admin from 123.45.67.89 port 57792 ssh2");

    REQUIRE(message);
    
    std::time_t raw = std::chrono::system_clock::to_time_t(message->entryDate);
    auto tmStruct = std::localtime(&raw);
    INFO(raw);

    INFO(std::put_time(tmStruct, "%b %d %T, %Y"));

    REQUIRE(tmStruct->tm_mon == 7);
    REQUIRE(tmStruct->tm_mday == 17);
    REQUIRE(tmStruct->tm_hour == 22);
    REQUIRE(tmStruct->tm_min == 17);
    REQUIRE(tmStruct->tm_sec == 44);

    REQUIRE(raw == 1660767464ll);

    REQUIRE(message->host == "sinon");
    REQUIRE(message->process == "sshd");
    REQUIRE(message->message == "Failed password for invalid user admin from 123.45.67.89 port 57792 ssh2");
}
