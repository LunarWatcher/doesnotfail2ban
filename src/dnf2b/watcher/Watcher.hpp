#pragma once

#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/infrastructure/HealthCheck.hpp"
#include "dnf2b/sources/Parser.hpp"
#include <cstdint>
#include <string>
#include <optional>

namespace dnf2b {

class Watcher {
private:
    std::optional<std::string> multiProcessID;

    uint16_t port;
    int limit;

    std::vector<Filter> filters;

public:
    Watcher(std::optional<std::string> multiProcessID, uint16_t port, int limit, const std::vector<Filter>& filters);

    virtual std::map<std::string, int> process(const std::vector<Message>& filteredMessages);

};

}
