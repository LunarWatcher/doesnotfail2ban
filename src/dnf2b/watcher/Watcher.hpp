#pragma once

#include "dnf2b/infrastructure/HealthCheck.hpp"
#include "dnf2b/sources/Parser.hpp"
#include <string>
#include <optional>

namespace dnf2b {

class Watcher : public HealthCheck {
private:
    std::optional<std::string> process;

    bool enabled;
    std::string resource;

    int port;
    int limit;

    long long bans;

public:
    Watcher(const nlohmann::json& conf);

    virtual void process(const std::vector<Message>& filteredMessages);

    virtual std::string checkHealth() override;

};

}
