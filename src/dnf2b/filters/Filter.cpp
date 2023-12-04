#include "Filter.hpp"
#include "spdlog/spdlog.h"

#include <dnf2b/static/Constants.hpp>
#include <regex>

namespace dnf2b {

Filter::Filter(const std::string& filterName) : filterName(filterName) {
    nlohmann::json conf;

}

std::optional<MatchResult> Filter::checkMessage(const Message& message) {

    for (auto pattern : patterns) {
        if (message.ip.empty() && pattern.find("${dnf2b.ip}") == std::string::npos) {
            spdlog::error("Pattern {} (from filter {}) has no IP field, and was used with a parser that failed to find an IP. Skipping...",
                          pattern, this->filterName);
            continue;
        }

        bool hasInlineIP = false;

        size_t pos = 0;
        std::string search = "${dnf2b.ip}";
        while ((pos = pattern.find(search, pos)) != std::string::npos) {
            pattern.replace(pos, search.length(), Constants::IP_SEARCH_GROUP);
            pos += Constants::IP_SEARCH_GROUP.length();

            hasInlineIP = true;
        }

        std::regex compiledPattern(pattern);

        std::smatch m;
        if (!std::regex_search(message.message, m, compiledPattern)) {
            // No match found.
            continue;
        }

        std::string ip = hasInlineIP ? m[1].str() : message.ip;

        return MatchResult {
            .badness = this->danger,
            .ip = ip
        };
    }

    return std::nullopt;
}

}
