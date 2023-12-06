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
};

}
