#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <optional>

#include "dnf2b/infrastructure/HealthCheck.hpp"
#include "nlohmann/json.hpp"

namespace dnf2b {

/**
 * Barebones message data, as parsed from a given resource.
 */
typedef struct {
    const std::chrono::system_clock::time_point entryDate;
    const std::string process;
    /**
     * Not entirely sure if this has any practical use.
     */
    const std::string host;
    const std::string message;
} Message;

class Context;

/**
 * Superclass for all parsers.
 *
 * As is tradition, this class is health checked. It's up to the implementations to figure out what that
 * means for any given parser.
 *
 * Additionally, the resources are not managed by the superclass; it's up to each parser to figure out how
 * to best administer resources.
 *
 * It's the runner's job to determine whether or not to close a resource. Parsers with closeable resources
 * only need to worry about providing a method for sane closure.
 *
 * It's also up to each parser to sort out caching.
 */
class Parser : public HealthCheck {
public:
    nlohmann::json config;
    /**
     * The name of the parser. The type is not explicitly stored in code, as this is handled
     * via inheritance.
     */
    const std::string parserName;


    Parser(const std::string& parserName);

    /**
     * Reloads the config. 
     *
     * Can be overridden to add additional post-clearing code, such as validating all open resources, and closing if necessary.
     */
    virtual void reload(Context& ctx);
    

    /**
     * Polls a given resource, and runs it through the parser if applicable.
     */
    virtual std::vector<Message> poll() = 0;

    virtual std::optional<Message> parse(const std::string& line);

    /**
     * Closes a resource, if applicable.
     */
    virtual void close(const std::string& resourceName) = 0;
};

}
