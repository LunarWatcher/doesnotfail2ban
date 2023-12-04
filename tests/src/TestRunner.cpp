#include "spdlog/spdlog.h"
#include <filesystem>
#include <stdexcept>
#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"
#include "dnf2b/static/Constants.hpp"

std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "./test-files/dnf2b";

int main(int argc, const char* argv[]) {
    if (!std::filesystem::exists(dnf2b::Constants::DNF2B_ROOT)) {
        spdlog::error("You must run tests with build/ as the cwd. If you already are, don't touch test-resources/. Let the build system generate it, and stay away from it :)");
        return -1;
    }
    return Catch::Session().run(argc, argv);
}
