#pragma once


#include "dnf2b/sources/Parser.hpp"
namespace dnf2b {

struct MatchResult {
    int badness;
    std::string ip;

};

class Filter {
private:
    int danger;
    std::vector<std::string> patterns;
public:
    const std::string filterName;

    Filter(const std::string& filterName);

    std::optional<MatchResult> checkMessage(const Message& message);

};

}
