#pragma once


#include "dnf2b/sources/Parser.hpp"
#include "dnf2b/util/PCRE.hpp"
#include <filesystem>

namespace dnf2b {

/**
 * Currently, this is just a thin wrapper around the result. In the future, this may be expanded with additional functionality.
 */
struct MatchResult {
    std::string ip;
    std::string groupId;
    bool error = false;
};

class Filter {
private:
    std::vector<Pattern> patterns;

    bool insensitive;
public:
    const std::string filterName;

    explicit Filter(const std::string& filterName);
    explicit Filter(const std::filesystem::path& path);

    std::optional<MatchResult> checkMessage(const Message& message, size_t* matchIdx = 0);
    static std::filesystem::path getPathFromFilterName(const std::string& filterName);

    const std::vector<Pattern>& getPatterns() { return patterns; }

};

}
