#include "Tester.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/sources/Parser.hpp"
#include "dnf2b/sources/ParserLoader.hpp"
#include "dnf2b/static/Constants.hpp"

#include <iostream>
#include <format>

namespace dnf2b {

void CLI::testLineMatches(const std::string& logLine, const std::string& parserName) {
    auto parser = ParserLoader::loadParser(parserName, "noop");
    if (parser == nullptr) {
        spdlog::error("Failed to find a parser named {}.", parserName);
        return;
    }
    auto message = parser->parse(logLine);
    if (!message) {
        spdlog::error("Failed to parse {} with {}", logLine, parserName);
        return;
    }
#define PADDING std::left << std::setw(20)
    std::cout << "Parsed " << logLine << " to:\n"
        << PADDING << "Message:" << message->message << "\n"
        << PADDING << "Date: " << std::format("{}", message->entryDate) << "\n"
        << PADDING << "IP: " << message->ip << "\n"
        << PADDING << "Host: " << message->host << "\n";

    std::cout << std::endl;

    std::cout << "Running against filters..." << std::endl;

    for (auto& file : std::filesystem::directory_iterator(dnf2b::Constants::DNF2B_ROOT / "filters")) {
        if (file.is_regular_file()) {
            auto path = file.path();
            if (path.extension() == ".json") {
                auto name = path
                    .replace_extension() // Drop .json extension
                    .filename() // Drop path
                    .string();
                if (name.starts_with("dummy")) {
                    continue;
                }

                dnf2b::Filter f(name);
                
                bool hasMatched = false;
                size_t offset = 0;
                const auto& patterns = f.getPatterns();
                while (f.checkMessage(*message, &offset).has_value()) {
                    if (!hasMatched) {
                        std::cout << "Match found in " << path << std::endl;
                        hasMatched = true;
                    }

                    std::cout << "\tMatched by pattern: " << patterns.at(offset++).getRawPattern() << std::endl;

                }
            }
        }
    }
}

}
