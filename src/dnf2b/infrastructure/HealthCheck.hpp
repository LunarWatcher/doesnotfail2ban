#pragma once

#include <string>

namespace dnf2b {

class HealthCheck {
public:
    virtual ~HealthCheck() = default;

    /**
     * Returns a string containing miscellaneous debug information.
     *
     * Namespacing is covered by the invoker, and does not have to be explicitly
     * added in any particular format.
     * The name of the provider may still be included, particularly for services
     * involving other executables
     */
    virtual std::string checkHealth() = 0;
};

}
