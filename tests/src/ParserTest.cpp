#include "catch2/catch_test_macros.hpp"

#include "dnf2b/sources/FileParser.hpp"
#include <iostream>

#include <time.h>

TEST_CASE("Make sure trivial parsing works", "[parser]") {
    dnf2b::FileParser p("journald");
    std::ifstream x("../etc/dnf2b/parsers/journald.json");
    INFO("If x.is_open() fails, this is a _solid_ indicator you're in the wrong cwd. cd into a build directory and try again");
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

TEST_CASE("Non-multiprocess parsing", "[parser]") {
    dnf2b::FileParser p("yourmom");
    // For this example, we use a non-multiprocess system
    // that uses this log format:
    //    [subsystem, not unique, and not constant] Month Day Time: message
    //
    // The date format is identical to the systemd example, because I'm lazy. Fight me.
    p.config = {
        {"type", "file"},
        {"multiprocess", false},
        {"pattern",
            {
                {"full", "^[^ ]+ ((?:[^ ]+ ?){3}): (.*)$"},
                {"time", "%b %d %T"},
                {"groups",
                    {
                        {"time", 0},
                        {"message", 1}
                    }
                }
            }
        }
    };

    auto message = p.parse("[core] Aug 17 21:22:23: message");

    REQUIRE(message);
    
    std::time_t raw = std::chrono::system_clock::to_time_t(message->entryDate);
    // Short process, let's just make sure the UNIX timestamp matches.
    REQUIRE(raw == 1660764143ll);
    REQUIRE(message->message == "message");

    REQUIRE(message->host == "");
    REQUIRE(message->process == "");

}
