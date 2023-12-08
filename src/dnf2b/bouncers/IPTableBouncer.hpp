#pragma once

#include "dnf2b/bouncers/Bouncer.hpp"
#include "dnf2b/data/Structs.hpp"

#include <nlohmann/json.hpp>

namespace dnf2b {

/**
 * This bouncer uses iptables to reject users. It supports port-specific blocks.
 *
 * Note that this bouncer uses `system()` to issue the commands, as iptables
 * does not provide a library with a stable API. The command is the only stable
 * option
 */
class IPTableBouncer : public Bouncer {
private:
    std::string strategy;
    bool useIpset;

    /**
     * Utility function that returns either "iptables" or "ip6tables",
     * depending on whether the provided IP is ipv4 or ipv6.
     *
     * This is because iptables is split into two commands, and which
     * to use is determined by the IP type.
     */
    std::string getCommand(const std::string& ip);
    std::string getRule(const std::string& ip);
    std::string getIpsetCommand(const std::string& ip, bool remove);
public:
    IPTableBouncer(const nlohmann::json& config);

    void ban(const std::string& ip, std::optional<uint16_t> port) override;
    void unban(const std::string& ip, std::optional<uint16_t> port) override;

    bool persistentBans() override { return false; }

};

}
