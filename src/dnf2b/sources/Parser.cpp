#include "Parser.hpp"

#include "dnf2b/core/Context.hpp"
#include "dnf2b/except/Exceptions.hpp"

#include <sstream>
#include <fstream>
#include <regex>

namespace dnf2b {

Parser::Parser(const std::string& parserName) : parserName(parserName) {}

void Parser::reload(Context& ctx) {
    std::ifstream in(std::filesystem::path("/etc/dnf2b/parsers/") / (parserName + ".json"));
    if (!in) {
        throw except::GenericException("Failed to load " + parserName + ": file not found.", 255);
    }

    in >> config;
    // TODO: validate config? Should also be merged into parse to relieve the validation done there. Would also allow more extensive validation.
    // Is there a built-in validation system?
    // As an aside, a dedicated validation function would also enable a validation command for parsers. Extending that to filters
    // and communicators is actually a stonks idea
}

std::optional<Message> Parser::parse(const std::string& line) {
    std::smatch match;
    auto& pattern = config.at("pattern");

    // TODO: sort out flags
    auto regex = std::regex{pattern["full"].get<std::string>()};
    auto timePattern = pattern["time"].get<std::string>();

    auto& groups = pattern["groups"];

    if (!groups.contains("time")) {
        throw except::GenericException("Required field \"time\" missing from " + parserName, 255);
    } else if (!groups.contains("message")) {
        throw except::GenericException("Required field \"message\" missing from " + parserName, 255);
    } else if (config["multiprocess"].get<bool>() && !groups.contains("process")) {
        throw except::GenericException("Required field \"process\" in multiprocess parser missing from " + parserName, 255);
    }

    if (std::regex_match(line, match, regex)) {

        auto timeString = match[groups["time"].get<int>() + 1].str();
        std::tm tm = {};
        std::stringstream ss(timeString);
        ss >> std::get_time(&tm, timePattern.c_str());
        return Message {
            // entryDate
            std::chrono::system_clock::from_time_t(std::mktime(&tm)),
            // process
            config["multiprocess"] ? match[groups["process"].get<int>() + 1].str() : "",
            // host
            groups.contains("host") ? match[groups["host"].get<int>() + 1].str() : "",
            // message
            match[groups["message"].get<int>() + 1].str()
        };

    }

    return std::nullopt;
}

}
