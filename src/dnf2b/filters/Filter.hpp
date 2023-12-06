#pragma once


#include "dnf2b/sources/Parser.hpp"
namespace dnf2b {

/**
 * Currently, this is just a thin wrapper around the result. In the future, this may be expanded with additional functionality.
 */
struct MatchResult {
    std::string ip;
};

class Filter {
private:
    std::vector<std::string> patterns;

    bool insensitive;
public:
    const std::string filterName;

    Filter(const std::string& filterName);

    std::optional<MatchResult> checkMessage(const Message& message);

};

}
