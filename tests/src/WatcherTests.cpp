#include "catch2/catch_test_macros.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/watcher/Watcher.hpp"

TEST_CASE("Validate processing logic", "[Watcher]") {
    dnf2b::Watcher watcher(
        "demo", 
        std::nullopt,
        69,
        0,
        {
            dnf2b::Filter("dummy-filter")
        }, 
        "global");
}
