#pragma once

#include "Bouncer.hpp"

namespace dnf2b {

class UFWBouncer : public Bouncer {
public:
    UFWBouncer();

    bool hasUFW();

    void unbanIP(const std::string& ip) override;
    void banIP(const std::string& ip) override;

    std::string checkHealth() override;
};

}
