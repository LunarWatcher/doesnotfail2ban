#include "BanManager.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/network_v4.hpp"
#include "asio/ip/network_v6.hpp"
#include "dnf2b/static/Constants.hpp"
#include "dnf2b/util/Parsing.hpp"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <variant>
#include "dnf2b/bouncers/BouncerLoader.hpp"
#include "dnf2b/json/Config.hpp"

namespace dnf2b {

BanManager::BanManager(ConfigRoot& ctx) : db(Constants::DNF2B_ROOT / "db.sqlite3"), conf(ctx) {
    auto rawWhitelist = ctx.core.whitelist;

    for (auto& entry : rawWhitelist) {
        spdlog::info("Whitelisting {}", entry);
        if (entry.find('/') == std::string::npos) {
            // Address
            auto addr = asio::ip::make_address(entry);
            if (addr.is_v4()) {
                whitelist.push_back(addr.to_v4());
            } else {
                whitelist.push_back(addr.to_v6());
            }

        } else {
            // Network
            if (entry.find(":") == std::string::npos) {
                // Network_v4
                whitelist.push_back(asio::ip::make_network_v4(entry));
            } else {
                // Network v6
                whitelist.push_back(asio::ip::make_network_v6(entry));
            }
        }
    }

    if (conf.bouncers.size() == 0) {
        spdlog::error("No bouncers configured. Shutting down");
        throw std::runtime_error("There's no point in running dnf2b without bouncers. Fix your config");
    }
    for (auto& [bouncer, config] : conf.bouncers.items()) {
        spdlog::info("Loading bouncer {}", bouncer);
        auto bouncerPtr = BouncerLoader::loadBouncer(bouncer, config);
        this->bouncers[bouncer] = bouncerPtr;
    }

}

void BanManager::log(Watcher* source, std::map<std::string, int> ipFailMap) {
    for (auto& [ip, timesCaught] : ipFailMap) {
        if (isWhitelisted(ip)) {
            spdlog::info(
                "{} was caught, but is whitelisted. Skipping...",
                ip
            );
            continue;
        }
        auto currTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::vector<double> fails(timesCaught, currTime);

        // This is probably shit for performance.
        auto info = getIpInfo(ip);

        if (info.currBans.contains(source->getBouncerName())) {
            spdlog::warn("{} is already banned banned by {}, but appears in the logs after the ban. Possible race condition (wontfix), or"
                         " reban failure (pretty big fucking problem). If this happened shortly after the ban (logs are not read in real-time), or shortly after startup,"
                         " it's probably fine. A reban will be issued for good measure", ip, source->getBouncerName());

            auto bouncer = bouncers.at(source->getBouncerName());
            bouncer->ban(ip, source->getPort());
            continue;
        }

        info.currFails.insert(info.currFails.end(), fails.begin(), fails.end());

        if ((info.currFails.size() > 0 && source->getFailThreshold() < 0) || (info.currFails.size() >= (size_t) source->getFailThreshold())) {
            spdlog::info("{} has received the threshold for {} and will be banned", ip, source->getId());
            auto bouncer = bouncers.at(source->getBouncerName());

            bouncer->ban(ip, source->getPort());

            info.currFails.clear();
            if (info.currBans.contains(source->getBouncerName())) {
                spdlog::warn("{} has already been banned in {}. Skipping re-ban; race condition?", ip, source->getBouncerName());
                continue;
            } else {
                std::optional<int64_t> duration =
                    conf.core.control.banPeriod < 0 ?
                        std::nullopt
                        : std::optional(
                            conf.core.control.banPeriod
                                * static_cast<int64_t>(std::pow(conf.core.control.banIncrement, info.banCount))
                        );
                // Overflow protection
                if (duration.has_value() && *duration < 0) {
                    // overflow, cap to max
                    duration = std::nullopt;
                }
                info.currBans[source->getBouncerName()] = {
                    currTime,
                    duration,
                    std::nullopt
                };
            }

            // When banned, get rid of any stored fails
            db.wipeFailsFor(info.ip);

            // Remove cache entries post-ban
            if (auto it = failCache.find(info.ip); it != failCache.end()) {
                failCache.erase(it);
            }

            info.banCount += 1;
        } else {
            // Failed, but not immediately banned, IPs are stored in a cache.
            failCache[ip] = info;
        }
        db.updateIp(info);
    }
}

void BanManager::loadRebans() {
    auto pending = db.getBannedMinusPendingUnbans();

    for (auto& ban : pending) {
        if (!this->bouncers.contains(ban.bouncer)) {
            spdlog::warn("{} was banned by {}, which is no longer loaded, and cannot be rebanned", ban.ip, ban.bouncer);
            continue;
        }
        spdlog::debug("Re-banning {}", ban.ip);
        auto bouncer = this->bouncers.at(ban.bouncer);
        bouncer->ban(ban.ip, ban.port);
    }

    hasReloadedBans = true;
}

void BanManager::checkUnbansAndCleanup() {

    auto pending = db.getPendingUnbans();
    if (pending.size() != 0) {
        for (auto& unban : pending) {
            if (!this->bouncers.contains(unban.bouncer)) {
                spdlog::warn("{} was banned by {}, which is no longer loaded, and cannot be unbanned", unban.ip, unban.bouncer);
                continue;
            }

            auto bouncer = this->bouncers.at(unban.bouncer);
            bouncer->unban(unban.ip, unban.port);
        }

        db.unbanAll(pending);
    }

}

bool BanManager::isWhitelisted(const std::string& ip) {
    // IP is guaranteed to be a valid IP at this point. We need to parse it to a variant, though.
    std::variant<asio::ip::address_v4, asio::ip::address_v6> processedIp;
    std::variant<asio::ip::network_v4, asio::ip::network_v6> processedNetwork;

    // This is why I'm using asio for parsing.
    // I get to do this instead of writing hundreds of lines, and reading both the ipv4 and v6 specs.
    // Fuck. That. Shit.
    // Granted, asio doesn't make this process pretty (requiring both an address and a network^1),
    // but it works.
    // 
    // ^1: Strictly speaking, it's possible to get an address from a network for /32 or /128, but not
    // sure if the address is regenerated, considering it isn't a reference (IIRC)
    auto addr = asio::ip::make_address(ip);
    if (addr.is_v4()) {
        processedIp = addr.to_v4();
        processedNetwork = asio::ip::make_network_v4(addr.to_v4(), 32);
    } else {
        processedIp = addr.to_v6();
        processedNetwork = asio::ip::make_network_v6(addr.to_v6(), 128);
    }

    for (auto& exempt : whitelist) {
        if (std::holds_alternative<asio::ip::network_v4>(exempt)) {
            if (std::holds_alternative<asio::ip::address_v6>(processedIp)) {
                continue;
            }
            auto ip = std::get<asio::ip::network_v4>(processedNetwork);
            auto network = std::get<asio::ip::network_v4>(exempt);

            if (ip.is_subnet_of(network)) {
                return true;
            }
        } else if (std::holds_alternative<asio::ip::address_v4>(exempt)) {
            if (std::holds_alternative<asio::ip::address_v6>(processedIp)) {
                continue;
            }


            if(std::get<asio::ip::address_v4>(exempt) == std::get<asio::ip::address_v4>(processedIp)) {
                return true;
            }
        } else if (std::holds_alternative<asio::ip::network_v6>(exempt)) {
            if (std::holds_alternative<asio::ip::address_v6>(processedIp)) {
                continue;
            }
            auto ip = std::get<asio::ip::network_v6>(processedNetwork);
            auto network = std::get<asio::ip::network_v6>(exempt);

            if (ip.is_subnet_of(network)) {
                return true;
            }
        } else if (std::holds_alternative<asio::ip::address_v6>(exempt)) {
            if (std::holds_alternative<asio::ip::address_v4>(processedIp)) {
                continue;
            }
            
            if(std::get<asio::ip::address_v6>(exempt) == std::get<asio::ip::address_v6>(processedIp)) {
                return true;
            }
        }
    }
    
    return false;
}

}
