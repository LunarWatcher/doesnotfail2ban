#pragma once

#include <string>
#include <vector>

namespace dnf2b {

struct IPInfo {
    std::string ip;
    std::vector<double> currFails;
    int banCount;
    double banStarted;
};


}
