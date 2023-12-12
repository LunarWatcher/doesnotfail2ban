#include "Filter.hpp"
#include "dnf2b/util/PCRE.hpp"
#include "pcre2.h"
#include "spdlog/spdlog.h"

#include <dnf2b/static/Constants.hpp>
#include <filesystem>
#include <ranges>
#include <regex>
#include <fstream>
#include <algorithm>

namespace dnf2b {

Filter::Filter(const std::string& filterName) : filterName(filterName) {
    //auto path = Constants::DNF2B_ROOT / "filters" / (filterName + ".json");
    auto path = getPathFromFilterName(filterName);
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

std::filesystem::path Filter::getPathFromFilterName(const std::string& rawFilterName) {
    if (rawFilterName == "") {
        spdlog::error("Filter names cannot be blank");
        throw std::runtime_error("Filter names cannot be blank");
    }
    

    if (rawFilterName.at(0) == '@') {
        // Custom repo path mode
        auto slash = rawFilterName.find("/");
        if (slash == std::string::npos) {
            spdlog::error("Explicit repo paths must be in the format @subfolder-of-the-custom-folder/filter-name");
            throw std::runtime_error("Config error");
        }
        // General validation
        if (rawFilterName.find("/") != std::string::npos
            && std::count(rawFilterName.begin(), rawFilterName.end(), '/') > 1) {
            throw std::runtime_error("Nope");
        }
        auto path = rawFilterName.substr(1, slash - 1);
        auto fileName = rawFilterName.substr(slash + 1);

        return Constants::DNF2B_ROOT / "custom" / path / "filters" / (fileName + ".json");
    } else {
        if (rawFilterName.find('/') != std::string::npos) {
            throw std::runtime_error("Nope");
        }
        auto path = Constants::DNF2B_ROOT / "filters" / (rawFilterName + ".json");
        if (!std::filesystem::exists(path)) {
            auto customRoot = Constants::DNF2B_ROOT / "custom";
            for (auto custom : std::filesystem::directory_iterator(customRoot)) {

                if (std::filesystem::is_directory(custom) && std::filesystem::exists(custom.path() / "filters")) {
                    // check for filter existence
                    auto filterPath = custom.path() / "filters" / (rawFilterName + ".json");
                    if (std::filesystem::exists(filterPath)) {
                        path = filterPath;
                        goto return_path;
                    }
                }
            }
            spdlog::error("Filter {} doesn't exist in builtins or custom folders", rawFilterName);
            throw std::runtime_error("Failed to find filter");
        }
return_path:;
        return path;
    }
}
}
