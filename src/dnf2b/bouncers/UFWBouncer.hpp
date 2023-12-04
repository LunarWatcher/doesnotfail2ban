#pragma once

#include "Bouncer.hpp"

namespace dnf2b {

class UFWBouncer : public Bouncer {
public:
    UFWBouncer();

    bool hasUFW();

    void unban(const std::string& ip, std::optional<uint16_t> port) override;
    void banIP(const std::string& ip, std::optional<uint16_t> port) override;

    std::string checkHealth() override;
};

}
