#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace dnf2b {

class Bouncer {
public:
    Bouncer() = default;
    virtual ~Bouncer() = default;

    virtual void unban(const std::string& ip, std::optional<uint16_t> port) = 0;
    virtual void ban(const std::string& ip, std::optional<uint16_t> port) = 0;

    /**
     * Whether or not the bans issued by this bouncer is persistent or not.
     * If true, dnf2b will not issue a new ban command when restarting.
     */
    virtual bool persistentBans() = 0;

};

}
