#include "spdlog/spdlog.h"
#include <filesystem>
#include <stdexcept>
#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"
#include "dnf2b/static/Constants.hpp"

#include <fstream>

std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "./test-files/dnf2b";

int main(int argc, const char* argv[]) {
    if (!std::filesystem::exists(dnf2b::Constants::DNF2B_ROOT)) {
        spdlog::error("You must run tests with build/ as the cwd. If you already are, don't touch test-resources/. Let the build system generate it, and stay away from it :)");
        return -1;
    }

    {
        std::ofstream f(dnf2b::Constants::DNF2B_ROOT / "filters/dummy-filter.json");
        f << R"(
        {
            "danger": 10,
            "patterns": [
                "^Never gonna give you up",
                "^Foxes <3$"
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
                "full": "^\\[([^\\]]+)\\]: (.*)$",
                "time": "%T",
                "groups": {
                    "time": 0,
                    "message": 1
                }
            }
        }
        )" << std::endl;
    }

    return Catch::Session().run(argc, argv);
}
