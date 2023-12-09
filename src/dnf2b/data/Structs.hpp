#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace dnf2b {


struct BanInfo {
    int64_t banStarted;
    int64_t banDuration;
    std::optional<uint16_t> port;
};

struct IPInfo {
    std::string ip;
    std::vector<int64_t> currFails;
    int banCount;
    std::map<std::string /* bouncerName */, BanInfo> currBans;
};


}
