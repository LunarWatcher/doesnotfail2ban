#pragma once

#include "dnf2b/bouncers/Bouncer.hpp"
#include "dnf2b/core/Context.hpp"
#include "dnf2b/util/IP.hpp"
#include "dnf2b/watcher/Watcher.hpp"

namespace dnf2b {

struct IPInfo {
    std::string ip;
    std::vector<long long> fails;

};

/**
 *
 */
class BanManager {
private:
    std::map<std::string /* bouncer name */, std::shared_ptr<Bouncer>> bouncers;
    std::map<std::string /* group */, std::string /* bouncer */> groupBouncerMap;

    std::map<std::string, IPInfo> ipMap;

    std::vector<IP> whitelist;

public:
    BanManager(const Context& ctx);

    void log(Watcher* source, std::map<std::string, int> ipFailMap);

};

}
