#include <iostream>
#include <stdexcept>

#include "dnf2b/static/Constants.hpp"
#include "dnf2b/ui/CLI.hpp"
#include "spdlog/cfg/helpers.h"
#include "spdlog/spdlog.h"
#include "stc/Environment.hpp"

std::filesystem::path dnf2b::Constants::DNF2B_ROOT = "/etc/dnf2b";

int main(int argc, const char* argv[]) {
    std::string envVal = stc::getEnv("SPDLOG_LEVEL", "info");
    spdlog::cfg::helpers::load_levels(envVal);

    try {
        return dnf2b::CLI::parse(argc, argv);
    } catch (const int& x) {
        // I'm abusing int throwing as an overcomplicated way to return the value,
        // because why not? It works, and it's the only way to do this in a system where
        // whether the errors warrant a restart is up to the specific error.
        //
        // Future me here: What the actual fuck was past me thinking?
        return x;
    } catch (const std::runtime_error& e) {
        spdlog::error("Runtime exception caught: {}", e.what());
        return 255;
    }
}
