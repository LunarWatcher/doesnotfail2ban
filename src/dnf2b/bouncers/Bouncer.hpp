#pragma once

#include <string>

#include "dnf2b/infrastructure/HealthCheck.hpp"

namespace dnf2b {

class Bouncer : public HealthCheck {
public:
    Bouncer();
    virtual ~Bouncer() = default;

    virtual void unbanIP(const std::string& ip) = 0;
    virtual void banIP(const std::string& ip) = 0;
};

}
