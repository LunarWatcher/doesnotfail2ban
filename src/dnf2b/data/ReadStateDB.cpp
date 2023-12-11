#include "ReadStateDB.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <fstream>
#include <mutex>

namespace dnf2b {

ReadStateDB::ReadStateDB(std::filesystem::path path) : path(path / "parser-read-state.jsondb") {
    std::ifstream f(this->path);
    if (!f) {
        spdlog::debug("No existing state found.");
        return;
    }

    f >> state;
}

void ReadStateDB::store(const std::string& parserName, const std::string& resourceName, const nlohmann::json& fieldValue) {
    std::lock_guard<std::mutex> _g(mutex);
    state[parserName][resourceName] = fieldValue;
}

std::optional<nlohmann::json> ReadStateDB::read(const std::string& parserName, const std::string& resourceName) {
    std::lock_guard<std::mutex> _g(mutex);
    //state[parserName][resourceName] = fieldValue;
    if (!state.contains(parserName)) {
        return std::nullopt;
    }
    
    auto parserData = state.at(parserName);
    if (!parserData.contains(resourceName)) {
        return std::nullopt;
    }

    auto result = parserData.at(resourceName);
    // For good measure, in case a reset is needed, null is the same as non-existent.
    // Returning a nullopt ensures the parsers can figure out the defaults on their own,
    // rather than having them manually check for null and nullopt separately 
    if (result.is_null()) {
        return std::nullopt;
    }
    return result;

}

void ReadStateDB::commit() {
    std::lock_guard<std::mutex> _g(mutex);
    std::ofstream f(path);
    if (!f) {
        spdlog::error("ReadStateDB failed to access {}", path.string());
        return;
    }

    f << state.dump(4);
}

}
