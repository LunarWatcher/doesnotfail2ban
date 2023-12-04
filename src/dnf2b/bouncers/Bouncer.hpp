#pragma once

#include <string>
#include <optional>

#include "dnf2b/infrastructure/HealthCheck.hpp"

namespace dnf2b {

class Bouncer : public HealthCheck {
public:
    Bouncer();
    virtual ~Bouncer() = default;


    virtual void unban(const std::string& ip, std::optional<uint16_t> port) = 0;
    virtual void ban(const std::string& ip, std::optional<uint16_t> port) = 0;
};

}
