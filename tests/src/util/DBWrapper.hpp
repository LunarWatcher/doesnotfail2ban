#pragma once

#include "dnf2b/data/BanDB.hpp"
#include "dnf2b/static/Constants.hpp"
#include <filesystem>
namespace tests::util {

inline std::filesystem::path DB_PATH = dnf2b::Constants::DNF2B_ROOT / "db.sqlite3";

class DBWrapper {
public:
    std::shared_ptr<dnf2b::BanDB> db;
    DBWrapper(bool createInstance = true) {
        std::filesystem::remove(DB_PATH);
        if (createInstance) {
            db = std::make_shared<dnf2b::BanDB>(DB_PATH);
        }
    }
    ~DBWrapper() {
        std::filesystem::remove(DB_PATH);
    }

    auto operator->() { return db; }

};

}
