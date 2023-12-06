#pragma once

#include "dnf2b/bouncers/Bouncer.hpp"
namespace dnf2b {

/**
 * Using real bouncers for testing is probably Not A Good Idea:tm:.
 * This  bouncer exists for those purposes, as well as certain (extremely niche)
 * use-cases where a no-op bouncer in production is useful.
 *
 * DO NOT USE unless you either:
 * 1. Are writing a unit test
 * 2. Know what you're doing
 */
class NoopBouncer : public Bouncer {
public:
    NoopBouncer() = default;

    void unban(const std::string& ip, std::optional<uint16_t> port) override {}
    void ban(const std::string& ip, std::optional<uint16_t> port) override {}
};

}
