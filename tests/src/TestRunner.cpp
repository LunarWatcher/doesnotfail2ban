#define CATCH_CONFIG_RUNNER

#include "spdlog/spdlog.h"
#include <filesystem>
#include <stdexcept>
#include "catch2/catch_session.hpp"
#include "dnf2b/static/Constants.hpp"

#include <fstream>

std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "./test-files/dnf2b";

int main(int argc, const char* argv[]) {
    if (!std::filesystem::exists(dnf2b::Constants::DNF2B_ROOT)) {
        spdlog::error("You must run tests with build/ as the cwd. If you already are, don't touch test-resources/. Let the build system generate it, and stay away from it :)");
        return -1;
    }

    spdlog::set_level(spdlog::level::debug);

    {
        std::ofstream f(dnf2b::Constants::DNF2B_ROOT / "filters/dummy-filter.json");
        f << R"(
        {
            "patterns": [
                "^Never gonna give you up",
                "^Foxes <3$",
                "^Snacc attacc"
            ]
        }
        )" << std::endl;
    }
    {
        std::ofstream f(dnf2b::Constants::DNF2B_ROOT / "parsers/dummy-parser.json");
        f << R"(
        {
            "type": "file",
            "multiprocess": false,
            "pattern": {
                "full": "^\\[(?<Time>[^\\]]+)\\]: (?<Msg>.*)$",
                "time": "%T"
            }
        }
        )" << std::endl;
    }

    return Catch::Session()
        .run(argc, argv);
}
