#pragma once

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
#include "dnf2b/watcher/Watcher.hpp"
#include "nlohmann/json.hpp"

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

    std::vector<std::variant<asio::ip::network_v4, asio::ip::network_v6, asio::ip::address_v4, asio::ip::address_v6>> whitelist;

public:
    BanManager(const nlohmann::json& config);

    void log(Watcher* source, std::map<std::string, int> ipFailMap);

    bool isWhitelisted(const std::string& ip);

};

}
