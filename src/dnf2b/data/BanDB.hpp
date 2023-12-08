#pragma once

#include "dnf2b/data/Structs.hpp"
#include <filesystem>
#include "SQLiteCpp/SQLiteCpp.h"
#include <stc/FileLock.hpp>

namespace dnf2b {

struct UnbanInfo {
    std::string ip;
    std::string bouncer;
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

    void unban(const std::string& ip, const std::string& bouncer);

    void forgiveFails(double failAge);
    void wipeFailsFor(const std::string& ip);

    std::vector<UnbanInfo> getPendingUnbans();

};

}
