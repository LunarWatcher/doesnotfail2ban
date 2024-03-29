#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <optional>

#include "dnf2b/util/PCRE.hpp"
#include "nlohmann/json.hpp"

namespace dnf2b {

/**
 * Barebones message data, as parsed from a given resource.
 */
struct Message {
    std::chrono::system_clock::time_point entryDate;
    /**
     * Not entirely sure if this has any practical use.
     */
    std::string host;
    std::string message;

    /**
     * Contains the IP. Note that this is not guaranteed to be populated, and is only
     * populated if the IP is not part of the message.
     */
    std::string ip;

    std::string process;
};


/**
 * Currently only contains the regex pattern used by the fallback search.
 */
struct IPFallbackSearch {
    /**
     * The regex pattern used to match messages.
     */
    Pattern p;

    std::optional<std::string> search(const Message& m);

};

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
class Parser {
protected:
    std::optional<Pattern> pattern;
    std::string timeFormat;

    std::optional<IPFallbackSearch> fallbackSearch;
public:
    nlohmann::json config;
    /**
     * The name of the parser. The type is not explicitly stored in code, as this is handled
     * via inheritance.
     */
    const std::string parserName;
    const std::string resourceName;
    const bool multiprocess;
    const std::string id;


    Parser(const std::string& parserName, const nlohmann::json& config, const std::string& resourceName);

    /**
     * Polls a given resource, and runs it through the parser if applicable.
     */
    virtual std::vector<Message> poll() = 0;

    virtual std::optional<Message> parse(const std::string& line);
    std::vector<Message> filterMessages(const std::string& serviceName, const std::vector<Message>& messages);

    /**
     * Closes a resource, if applicable.
     */
    virtual void close(const std::string& resourceName) = 0;
    virtual bool enableBuffer() { return fallbackSearch.has_value(); }
};

}
