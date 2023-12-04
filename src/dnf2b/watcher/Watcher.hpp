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
    /**
     * Watcher ID. Used for internal identification purposes.
     *
     * Should be unique, shouldn't be changed, doesn't need to have any relation to the watcher itself.
     * This is, in certain ways, a glorified comment field.
     */
    std::string id;
    /**
     * Group IDs define where to look for occurrences before throwing an IP in jail. By default,
     * this is set to "global" to recommend one
     */
    std::string groupId;
    std::optional<std::string> multiProcessID;

    uint16_t port;
    int limit;

    std::vector<Filter> filters;

public:
    Watcher(
        const std::string& id,
        std::optional<std::string> multiProcessID,
        uint16_t port,
        int limit,
        const std::vector<Filter>& filters,
        const std::string& groupId
    );

    virtual std::map<std::string, int> process(const std::vector<Message>& filteredMessages);

};

}
