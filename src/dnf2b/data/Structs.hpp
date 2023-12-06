#pragma once

#include <optional>
#include <string>
#include <vector>

namespace dnf2b {

struct IPInfo {
    std::string ip;
    std::vector<double> currFails;
    int banCount;
    std::optional<double> banStarted;
};


}
