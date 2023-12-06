#include "IPTableBouncer.hpp"
#include "asio/ip/address.hpp"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

#include <cstdlib>
#include <stc/Environment.hpp>
#include <nlohmann/json.hpp>

namespace dnf2b {

IPTableBouncer::IPTableBouncer(const nlohmann::json& config) {
    strategy = config.value("strategy", "DROP");
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

}

void IPTableBouncer::ban(const std::string& ip, std::optional<uint16_t>) {
    auto rule = getRule(ip);
    auto code = std::system(fmt::format("{} -A dnf2b {}", getCommand(ip), rule).c_str());
    if (code != 0) {
        spdlog::error("iptables ban of {} failed. See previous logs");
        return;
    }
    spdlog::info("iptables: {} has been banned", ip);
}

void IPTableBouncer::unban(const std::string& ip, std::optional<uint16_t>) {
    auto rule = getRule(ip);

    auto code = std::system(fmt::format("{} -D dnf2b {}", getCommand(ip), rule).c_str());
    if (code != 0) {
        spdlog::error("iptables ban of {} failed. See previous logs");
        return;
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

    return fmt::format("-s {} -j {}", ip, strategy);
}

}
