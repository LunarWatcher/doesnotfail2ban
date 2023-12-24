#include "catch2/catch_test_macros.hpp"

#include "dnf2b/sources/FileParser.hpp"
#include "dnf2b/sources/Parser.hpp"
#include "dnf2b/sources/ParserLoader.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>

#include <time.h>

TEST_CASE("Non-multiprocess parsing", "[parser]") {
    nlohmann::json config = {
        {"type", "file"},
        {"multiprocess", false},
        {"pattern",
            {
                {"full", "^[^ ]+ (?<Time>(?:[^ ]+ ?){3}): (?<Msg>.*)$"},
                {"time", "%b %d %T"}
            }
        }
    };
    dnf2b::FileParser p("yourmom", config, "unused");
    // For this example, we use a non-multiprocess system
    // that uses this log format:
    //    [subsystem, not unique, and not constant] Month Day Time: message
    //
    // The date format is identical to an old systemd example, because I'm lazy. Fight me.

    auto message = p.parse("[core] Aug 17 21:22:23: message");

    REQUIRE(message);

    //Optimally, this would be tested, but doing so is a fucking pain in the ass
    //std::time_t raw = std::chrono::system_clock::to_time_t(message->entryDate);

    REQUIRE(message->message == "message");

    REQUIRE(message->host == "");
    REQUIRE(message->process == "");
}

TEST_CASE("Validate file update tracking", "[parser]") {
    auto rawParser = dnf2b::ParserLoader::loadParser("dummy-parser", "./file-parser-test.txt");
    dnf2b::FileParser& parser = *std::static_pointer_cast<dnf2b::FileParser>(rawParser);

    std::filesystem::remove("./file-parser-test.txt");

    std::ofstream f("./file-parser-test.txt");
    REQUIRE(f.is_open());
    REQUIRE(std::filesystem::exists("./file-parser-test.txt"));

    auto res = parser.poll();
    REQUIRE(res.size() == 0);

    f << "[12:34:56]: I like trains" << std::endl;

    res = parser.poll();
    REQUIRE(res.size() == 1);
    REQUIRE(res.at(0).message == "I like trains");

    f << "[12:34:56]: No, _I_ like trains" << std::endl;

    res = parser.poll();
    REQUIRE(res.size() == 1);
    REQUIRE(res.at(0).message == "No, _I_ like trains");


}
