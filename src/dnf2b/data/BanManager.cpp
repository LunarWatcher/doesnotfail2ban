#include "BanManager.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/network_v4.hpp"
#include "asio/ip/network_v6.hpp"
#include <variant>

namespace dnf2b {

BanManager::BanManager(const nlohmann::json& ctx) {
    auto rawWhitelist = ctx.at("core").value("whitelist", std::vector<std::string>{});

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

}

bool BanManager::isWhitelisted(const std::string& ip) {
    // IP is guaranteed to be a valid IP at this point. We need to parse it to a variant, though.
    std::variant<asio::ip::address_v4, asio::ip::address_v6> processedIp;
    std::variant<asio::ip::network_v4, asio::ip::network_v6> processedNetwork;

    // This is why I'm using asio for parsing.
    // I get to do this instead of writing hundreds of lines, and reading both the ipv4 and v6 specs.
    // Fuck. That. Shit.
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
