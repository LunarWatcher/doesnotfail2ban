#include "BanManager.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/network_v4.hpp"
#include "asio/ip/network_v6.hpp"
#include "dnf2b/static/Constants.hpp"
#include "spdlog/spdlog.h"
#include <chrono>
#include <limits>
#include <variant>
#include "dnf2b/bouncers/BouncerLoader.hpp"

namespace dnf2b {

BanManager::BanManager(const nlohmann::json& ctx) : db(Constants::DNF2B_ROOT / "db.sqlite3"){
    auto rawWhitelist = ctx.at("core").value("whitelist", std::vector<std::string>{});
    auto forgiveRaw = ctx.at("core").value("foregiveAfter", "never");

    for (auto& entry : rawWhitelist) {
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

    if (!ctx.contains("bouncers") || ctx.at("bouncers").size() == 0) {
        spdlog::error("No bouncers configured. Shutting down");
        throw std::runtime_error("There's no point in running dnf2b without bouncers. Fix your config");
    }
    for (auto& [bouncer, config] : ctx.at("bouncers").items()) {
        spdlog::info("Loading bouncer {}", bouncer);
        auto bouncerPtr = BouncerLoader::loadBouncer(bouncer, config);
        this->bouncers[bouncer] = bouncerPtr;
    }

}

void BanManager::log(Watcher* source, std::map<std::string, int> ipFailMap) {
    for (auto& [ip, timesCaught] : ipFailMap) {
        if (isWhitelisted(ip)) {
            spdlog::info("{} was caught, but is whitelisted. Skipping...");
            continue;
        }
        auto currTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::vector<double> fails(timesCaught, currTime);

        // This is probably shit for performance.
        auto info = getIpInfo(ip);
        info.currFails.insert(info.currFails.end(), fails.begin(), fails.end());

        if (info.currFails.size() >= source->getFailThreshold()) {
            spdlog::info("{} has received the threshold for {} and will be banned", ip, source->getId());
            auto bouncer = bouncers.at(source->getBouncerName());

            bouncer->ban(ip, source->getPort());

            info.currFails.clear();
            if (info.currBans.contains(source->getBouncerName())) {
                spdlog::warn("{} has already been banned in {}. Skipping re-ban; race condition?", ip, source->getBouncerName());
            } else {
                int64_t duration = banDuration < 0 ? -1 : banDuration * static_cast<int64_t>(std::pow(banIncrement, info.banCount));
                if (duration < 0) {
                    // overflow, cap to max
                    duration = -1;
                }
                info.currBans[source->getBouncerName()] = {
                    currTime,
                    duration
                };
            }

            // When banned, get rid of any stored fails
            db.wipeFailsFor(info.ip);
        } else {
            // Failed, but not immediately banned, IPs are stored in a cache.
            failCache[ip] = info;
        }
        db.updateIp(info);
    }
}

void BanManager::checkUnbansAndCleanup() {

    auto pending = db.getPendingUnbans();
    if (pending.size() != 0) {
        for (auto& unban : pending) {
            if (!this->bouncers.contains(unban.bouncer)) {
                spdlog::warn("{} was banned by {}, which is no longer loaded", unban.ip, unban.bouncer);
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
