#pragma once

#include "dnf2b/static/Constants.hpp"
#include <filesystem>
#include <mutex>
#include <nlohmann/json.hpp>

namespace dnf2b {


/**
 * This is a JSON-based meta database for storing read states.
 * Read states are parser-defined values used to determine where
 * to continue parsing. 
 *
 * DO NOT EDIT THE GENERATED FILE BY HAND! 
 * It can be deleted to force a complete rescan, but editing the
 * file can and will result in unexpected behaviour.
 *
 * Forcing a rescan alone comes with some of these consequences.
* Particularly, rescanning logs means old failed attempts will
 * be registered as a new fail, even if that isn't the case.
 * This can result in premature bans, and if you haven't configured
 * your whitelist properly, it can get you banned if you happen to
 * have generated a log entry matching a filter.
 */
class ReadStateDB {
private:
    std::mutex mutex;
    nlohmann::json state;

    std::filesystem::path path;
public:
    ReadStateDB(std::filesystem::path path);
    ~ReadStateDB() {
        commit();
    }

    /**
     * Stores a value in the database.
     * The parser and resource names must be supplied without modifications.
     * These are used for lookup purposes. The fieldValue can be anything,
     * however, as long as it can be represented in a JSON object.
     *
     * 
     */
    void store(
        const std::string& parserName,
        const std::string& resourceName,
        const nlohmann::json& fieldValue);
    std::optional<nlohmann::json> read(const std::string& parserName, const std::string& resourceName);

    void commit();
    static ReadStateDB& getInstance() {
        static ReadStateDB db(Constants::DNF2B_ROOT);
        return db;
    }
};

}
