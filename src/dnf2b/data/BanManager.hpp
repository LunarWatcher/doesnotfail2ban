#pragma once

#include "Structs.hpp"

#include "dnf2b/bouncers/Bouncer.hpp"
#include "dnf2b/core/Context.hpp"
#include <variant>
/**
 * This is _complete_ overkill; importing the entirety of asio
 * for one function and four classes is stupid, but I'm not 
 * reading up on the IPv4 and v6 specs just to 
 * deal with subnet parsing.
 *
 * Contributions to do so are welcome, but there's no way in hell I'm doing it.
 * Maybe one day, but not in the near nor distant foreseeable future.
 */
#include <asio.hpp>
#include "dnf2b/data/BanDB.hpp"
#include "dnf2b/watcher/Watcher.hpp"
#include "nlohmann/json.hpp"

namespace dnf2b {

/**
 *
 */
class BanManager {
private:
    std::map<std::string /* bouncer name */, std::shared_ptr<Bouncer>> bouncers;

    std::vector<std::variant<asio::ip::network_v4, asio::ip::network_v6, asio::ip::address_v4, asio::ip::address_v6>> whitelist;
    bool hasReloadedBans = false;

    BanDB db;
    long long banDuration, banIncrement, forgetAfter;

    std::map<std::string, IPInfo> failCache;

    IPInfo getIpInfo(const std::string& ip) {
        if (failCache.contains(ip)) {
            return failCache.at(ip);
        }
        return db.loadIp(ip);
    } 
    void loadBouncerRules();
    void loadRebans();

public:
    BanManager(const nlohmann::json& config);

    void log(Watcher* source, std::map<std::string, int> ipFailMap);
    void checkUnbansAndCleanup();
     
    bool isWhitelisted(const std::string& ip);
    friend class Daemon;
};

}
