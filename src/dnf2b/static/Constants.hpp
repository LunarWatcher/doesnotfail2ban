#pragma once

#include <filesystem>
namespace dnf2b::Constants {

/**
 * Defines where to look for config files.
 *
 * Outside tests, this is set to /etc/dnf2b in Main.cpp. In tests, it's set to a local directory instead.
 */
extern std::filesystem::path DNF2B_ROOT;

/**
 * In an optimal world, this should probably be a better regex aimed at actually validating that the extracted text
 * is an IP, and not accidentally something else that was globbed in. I don't really see that as necessary.
 * Consumer regexes should anchor themselves properly instead.
 */
const static std::string IP_SEARCH_GROUP = "(?<IP>\\S+)";

}
