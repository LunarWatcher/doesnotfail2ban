#include "ParserLoader.hpp"
#include "dnf2b/sources/FileParser.hpp"
#include "dnf2b/sources/JournalCTLParser.hpp"
#include "dnf2b/static/Constants.hpp"
#include "spdlog/spdlog.h"

#include <fstream>
#include <filesystem>

namespace dnf2b {

std::shared_ptr<Parser> ParserLoader::loadParser(const std::string &parserName, const std::string& resourceName) {
    auto path = Constants::DNF2B_ROOT / "parsers" / (parserName + ".json");
    std::ifstream in(path);
    if (!in) {
        spdlog::error("Failed to load parser {} ({})", parserName, path.string());
        return nullptr;
    }
    nlohmann::json config;
    in >> config;

    auto type = config.at("type").get<std::string>();
    if (type == "file") {
        return std::make_shared<FileParser>(parserName, config, resourceName);
    } else if (type == "journalctl") {
        return std::make_shared<JournalCTLParser>(parserName, config, resourceName);
    } else {
        spdlog::error("Type {} is not valid", type);
        throw std::runtime_error("Configuration error: type is not valid");
    }

}

}
