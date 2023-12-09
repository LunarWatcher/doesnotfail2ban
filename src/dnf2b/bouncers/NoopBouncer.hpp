#pragma once

#include "dnf2b/bouncers/Bouncer.hpp"
#include <functional>
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
    using FuncProxy = std::function<void(const std::string&, std::optional<uint16_t>)>;
    static inline FuncProxy _ban = nullptr, _unban = nullptr;
    NoopBouncer() = default;
    ~NoopBouncer() {
        // reset the ban and unban proxies so tests don't die
        _ban = nullptr;
        _unban = nullptr;
    }

    void unban(const std::string& ip, std::optional<uint16_t> port) override {
        if (_unban) _unban(ip, port);
    }
    void ban(const std::string& ip, std::optional<uint16_t> port) override {
        if (_ban) _ban(ip, port);
    }

    bool persistentBans() override { return false; }
};

}
