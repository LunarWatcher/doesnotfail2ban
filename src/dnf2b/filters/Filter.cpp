#include "Filter.hpp"
#include "dnf2b/util/PCRE.hpp"
#include "pcre2.h"
#include "spdlog/spdlog.h"

#include <dnf2b/static/Constants.hpp>
#include <regex>
#include <fstream>

namespace dnf2b {

Filter::Filter(const std::string& filterName) : filterName(filterName) {
    auto path = Constants::DNF2B_ROOT / "filters" / (filterName + ".json");
    std::ifstream in(path);
    if (!in) {
        spdlog::error("Failed to load filter {} ({})", filterName, path.string());
        throw std::runtime_error("Config error");
    }
    nlohmann::json config;
    in >> config;

    std::vector<std::string> strPatterns = config.at("patterns");
    std::transform(strPatterns.cbegin(), strPatterns.cend(), std::back_inserter(patterns), 
        [](auto pattern) {
        size_t pos = 0;
        while ((pos = pattern.find("${dnf2b.ip}", pos)) != std::string::npos) {
            pattern.replace(pos, strlen("${dnf2b.ip}"), Constants::IP_SEARCH_GROUP);
            pos += Constants::IP_SEARCH_GROUP.length();

        }
        return Pattern(pattern, PCRE2_CASELESS | PCRE2_UTF);
        }
    );
    insensitive = config.value("insensitive", false);
}

std::optional<MatchResult> Filter::checkMessage(const Message& message) {

    for (const auto& pattern : patterns) {
        PCREMatcher matcher(pattern, message.message);
        if (!matcher.next()) {
            // No match found.
            continue;
        }

        std::string ip = matcher.get("IP").value_or(message.ip);
        if (ip == "") {
            spdlog::error("Filter {} lacks ${{dnf2b.ip}}, but was used with a parser that doesn't find an IP outside the message", this->filterName);
            throw std::runtime_error("Filter syntax error");
        }

        return MatchResult {
            .ip = ip
        };
    }

    return std::nullopt;
}

}
