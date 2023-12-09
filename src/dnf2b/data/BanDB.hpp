#pragma once

#include "dnf2b/data/Structs.hpp"
#include <filesystem>
#include "SQLiteCpp/SQLiteCpp.h"
#include <stc/FileLock.hpp>

namespace dnf2b {

struct UnbanInfo {
    std::string ip;
    std::string bouncer;
    std::optional<uint16_t> port;
};

class BanDB {
private:
    SQLite::Database db;

    std::filesystem::path dbLoc;
    std::mutex lock;
public:
    BanDB(std::filesystem::path dbLoc);

    void updateIp(const IPInfo& source);
    bool isBanned(const std::string& ip, const std::string& bouncer);
    IPInfo loadIp(const std::string& ip);

    /**
     * Removes an entry from the BanRegistry with a corresponding ip and bouncer.
     *
     * Has an additional parameter (subprocess), but this should not be used under normal conditions.
     * Setting this bool to true prevents the function from acquiring the mutex, which can lead
     * to race conditions.
     *
     * If set to true, the calling function can and absolutely should acquire the lock
     * independently.
     */
    void unban(const std::string& ip, const std::string& bouncer, bool subprocess = false);
    void unbanAll(const std::vector<UnbanInfo>& entries);

    void forgiveFails(double failAge);
    void wipeFailsFor(const std::string& ip);

    std::vector<UnbanInfo> getPendingUnbans();

};

}
