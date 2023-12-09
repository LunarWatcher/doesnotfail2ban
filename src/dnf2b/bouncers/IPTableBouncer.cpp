#include "IPTableBouncer.hpp"
#include "asio/ip/address.hpp"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

#include <cstdlib>
#include <stc/Environment.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dnf2b {

IPTableBouncer::IPTableBouncer(const nlohmann::json& config) {
    strategy = config.value("strategy", "DROP");
    useIpset = config.value("ipset", false);
    if (strategy != "DROP" && strategy != "REJECT") {
        spdlog::error("{} is not a valid strategy", strategy);
        throw std::runtime_error("Unsupported strategy: " + strategy);
    }

    spdlog::info("Initialising iptables...");
    // For some reason, ipv4 and ipv6 are split
    std::vector<std::string> commands = { "iptables", "ip6tables" };
    for (auto& command : commands) {
        spdlog::info("Preparing {}...", command);

        int code = 0;
        auto output = stc::syscommand(command + " -N dnf2b", &code);
        if (code != 0) {
            if (output.find("Chain already exists") == std::string::npos) {
                spdlog::info("{} already initialised. Purging chain", command);
                stc::syscommand(fmt::format("{} -F dnf2b", command));
            } else {
                spdlog::error("{} -N failed unexpectedly: {}", command, output);
                throw std::runtime_error("Failed to initialise iptables");
            }
        }
    }

    if (useIpset) {
        if (std::system("ipset create dnf2b-v4-blacklist hash:ip hashsize 4096 family inet") != 0) {
            spdlog::error("ipset failed");
            throw std::runtime_error("Failed to create ipset");
        }
        if (std::system("ipset create dnf2b-v6-blacklist hash:ip hashsize 4096 family inet6") != 0) {
            spdlog::error("ipset failed");
            throw std::runtime_error("Failed to create ipset");
        }

        auto code = std::system(fmt::format("iptables -A dnf2b --match-set dnf2b-v4-blacklist -j {}", strategy).c_str());
        if (code != 0) {
            spdlog::error("iptables failed to set up ipset rule");
            throw std::runtime_error("Ipset rule fail");
        }
        code = std::system(fmt::format("ip6tables -A dnf2b --match-set dnf2b-v6-blacklist -j {}", strategy).c_str());
        if (code != 0) {
            spdlog::error("iptables failed to set up ipset rule");
            throw std::runtime_error("Ipset rule fail");
        }
    }
}

void IPTableBouncer::ban(const std::string& ip, std::optional<uint16_t>) {
    if (useIpset) {
        auto command = getIpsetCommand(ip, false);
        auto code = std::system(fmt::format("{} {}", command, ip).c_str());
        if (code != 0) {
            spdlog::error("ipset ban of {} failed.", ip);
            return;
        }
    } else {
        auto rule = getRule(ip);
        auto code = std::system(fmt::format("{} -A dnf2b {}", getCommand(ip), rule).c_str());
        if (code != 0) {
            spdlog::error("iptables ban of {} failed. See previous logs");
            return;
        }
    }
    spdlog::info("iptables: {} has been banned", ip);
}

void IPTableBouncer::unban(const std::string& ip, std::optional<uint16_t>) {
    if (useIpset) {
        auto command = getIpsetCommand(ip, true);
        auto code = std::system(fmt::format("{} {}", command, ip).c_str());
        if (code != 0) {
            spdlog::error("ipset unban of {} failed.", ip);
            return;
        }
    } else {
        auto rule = getRule(ip);

        auto code = std::system(fmt::format("{} -D dnf2b {}", getCommand(ip), rule).c_str());
        if (code != 0) {
            spdlog::error("iptables unban of {} failed. See previous logs");
            return;
        }
    }
    spdlog::info("iptables: {} has been unbanned", ip);
}

std::string IPTableBouncer::getCommand(const std::string& ip) {
    auto addr = asio::ip::make_address(ip);
    if (addr.is_v4()) {
        return "iptables";
    } else {
        return "ip6tables";
    }
}

std::string IPTableBouncer::getRule(const std::string& ip) {

    return fmt::format("-n -s {} -j {}", ip, strategy);
}


std::string IPTableBouncer::getIpsetCommand(const std::string& ip, bool remove) {
    auto command = std::string("ipset ") + (remove ? "del" : "add") + " ";
    if (ip.find(":") != std::string::npos) {
        command += "dnf2b-v6-blacklist";
    } else {
        command += "dnf2b-v4-blacklist";
    }
    return command;
}

}
