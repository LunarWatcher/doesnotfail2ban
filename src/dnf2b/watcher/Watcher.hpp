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
    std::optional<std::string> multiProcessID;

    std::optional<uint16_t> port;
    int limit;

    std::vector<Filter> filters;
    std::string bouncer;

public:
    Watcher(
        const std::string& id,
        std::optional<std::string> multiProcessID,
        std::optional<uint16_t> port,
        int limit,
        const std::vector<Filter>& filters,
        const std::string& bouncer
    );
    virtual ~Watcher() = default;

    virtual std::map<std::string, int> process(const std::vector<Message>& filteredMessages);

    const std::string& getId() { return id; }
    const std::string& getBouncerName() { return bouncer; }
    const std::optional<std::string>& getProcessID() { return multiProcessID; }
    int getFailThreshold() { return limit; }
    decltype(port) getPort() { return port; }
};

}
