#include "BanDB.hpp"
#include "SQLiteCpp/Column.h"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"
#include "SQLiteCpp/Transaction.h"
#include "dnf2b/data/Structs.hpp"

namespace dnf2b {

BanDB::BanDB(std::filesystem::path dbLoc) : db(dbLoc, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE), dbLoc(dbLoc) {
    // Fuck you future me, I'm not reimplementing migrations. Have fun when you inevitably
    // need to change the database schema and need to clean up after my mess
    db.exec(R"(
    CREATE TABLE IF NOT EXISTS IPRegistry (
        IP TEXT PRIMARY KEY,
        BanCount NUMBER NOT NULL
    );
    )");
    db.exec(R"(
    CREATE TABLE IF NOT EXISTS FailRegistry (
        IP TEXT NOT NULL,
        Time NUMBER NOT NULL,
        UNIQUE(IP, Time)
    );
    )");
    db.exec(R"(
    CREATE TABLE IF NOT EXISTS BanRegistry (
        IP TEXT NOT NULL,
        Bouncer TEXT NOT NULL,
        BanStarted NUMBER NOT NULL,
        BanDuration NUMBER,
        Port NUMBER,
        UNIQUE(IP, Bouncer)
    );
    )");

    // Make SQLite more speeeed 
    db.exec("CREATE INDEX IF NOT EXISTS ipip ON IPRegistry (IP)");
    db.exec("CREATE INDEX IF NOT EXISTS failip ON FailRegistry (IP)");
    db.exec("CREATE INDEX IF NOT EXISTS banip ON BanRegistry (IP)");
}

void BanDB::forgiveFails(double failAge) {
    std::lock_guard g(lock);
    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto forgiveBefore = currTime - failAge;

    SQLite::Statement s(db, R"(
    DELETE FROM FailRegistry WHERE Time < ?
    )");
    s.bind(1, forgiveBefore);
    s.exec();
}

void BanDB::wipeFailsFor(const std::string& ip) {
    std::lock_guard g(lock);
    SQLite::Statement s(db, R"(
    DELETE FROM FailRegistry WHERE IP = ?
    )");
    s.bind(1, ip);
    s.exec();
}

bool BanDB::isBanned(const std::string& ip, const std::string& bouncer) {
    std::lock_guard g(lock);
    SQLite::Statement q(db, "SELECT EXISTS(SELECT 1 FROM BanRegistry WHERE IP = ? AND Bouncer = ?)");
    q.bind(1, ip);
    q.bind(2, bouncer);
    q.executeStep();
    // No step means no entry in the database, so the IP cannot be banned.
    return q.getColumn(0).getInt();
}

IPInfo BanDB::loadIp(const std::string& ip) {
    std::lock_guard g(lock);
    IPInfo info;
    // Because duh
    info.ip = ip;

    SQLite::Statement q(db, "SELECT BanCount FROM IPRegistry WHERE IP = ?");
    q.bind(1, ip);

    // Load base info
    if (q.executeStep()) {
        info.banCount = q.getColumn(0);
    }

    // Load fails
    SQLite::Statement fq(db, "SELECT Time FROM FailRegistry WHERE IP = ?");
    fq.bind(1, ip);
    while (fq.executeStep()) {
        info.currFails.push_back(fq.getColumn(0).getInt64());
    }

    // Load bans
    SQLite::Statement bq(db, "SELECT Bouncer, BanStarted, BanDuration, Port FROM BanRegistry WHERE IP = ?");
    bq.bind(1, ip);
    while (bq.executeStep()) {
        info.currBans[bq.getColumn(0)] = {
            bq.getColumn(1).getInt64(),
            bq.getColumn(2).isNull() ? std::nullopt : std::optional(bq.getColumn(2).getInt64()),
            bq.getColumn(3).isNull() ? std::nullopt : std::optional<uint16_t>(static_cast<uint16_t>(bq.getColumn(3).getInt()))
        };
    }

    return info;
}

void BanDB::updateIp(const IPInfo& source) {
    std::lock_guard g(lock);
    SQLite::Transaction t(db);

    SQLite::Statement s(db, R"(
    INSERT OR REPLACE INTO IPRegistry (IP, BanCount) VALUES (?, ?);
    )");
    s.bind(1, source.ip);
    s.bind(2, source.banCount);
    s.exec();

    for (auto& fail : source.currFails) {
        SQLite::Statement s(db, R"(
        INSERT OR REPLACE INTO FailRegistry (IP, Time) VALUES (?, ?)
        )");
        s.bind(1, source.ip);
        s.bind(2, fail);
        s.exec();
    }

    for (auto& [bouncer, banInfo] : source.currBans) {
        SQLite::Statement s(db, R"(
        INSERT OR REPLACE INTO BanRegistry (IP, Bouncer, BanStarted, BanDuration, Port) VALUES (?, ?, ?, ?, ?);
        )");
        s.bind(1, source.ip);
        s.bind(2, bouncer);
        s.bind(3, banInfo.banStarted);
        banInfo.banDuration.has_value() ? s.bind(4, *banInfo.banDuration) : s.bind(4);
        banInfo.port.has_value() ? s.bind(5, *banInfo.port) : s.bind(5);

        s.exec();
    }

    t.commit();
}

void BanDB::unban(const std::string& ip, const std::string& bouncer, bool subprocess) {
    if (!subprocess) {
        std::lock_guard g(lock);
    }
    SQLite::Statement s(db, R"(
    DELETE FROM BanRegistry WHERE IP = ? AND Bouncer = ?
    )");
    s.bind(1, ip);
    s.bind(2, bouncer);
    s.exec();
}

void BanDB::unbanAll(const std::vector<BanLocationInfo>& items) {
    std::lock_guard g(lock);

    // A transaction is necessary to improve performance. Without it, this risks writing
    // items.size() times, which comes with significant overhead. Writing once is more speed
    SQLite::Transaction t(db);
    for (auto& info : items) {
        this->unban(info.ip, info.bouncer, true);
    }
    t.commit();
}

std::vector<BanLocationInfo> BanDB::getPendingUnbans() {
    std::lock_guard g(lock);

    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    SQLite::Statement s(db, R"(
    SELECT IP, Bouncer, Port, BanStarted, BanDuration, (BanStarted + BanDuration)
    FROM BanRegistry
    WHERE
        BanDuration >= 0
    AND
        (BanStarted + BanDuration) <= ?
    )");
    s.bind(1, currTime);

    std::vector<BanLocationInfo> out;
    while (s.executeStep()) {
        out.push_back({
            .ip = s.getColumn(0),
            .bouncer = s.getColumn(1),
            .port = s.getColumn(2).isNull() ? std::nullopt : std::optional(s.getColumn(2))
        });
    }
    
    return out;
}
std::vector<BanLocationInfo> BanDB::getBannedMinusPendingUnbans() {

    std::lock_guard g(lock);

    auto currTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    SQLite::Statement s(db, R"(
    SELECT IP, Bouncer, Port, BanStarted, BanDuration, (BanStarted + BanDuration)
    FROM BanRegistry
    WHERE
    (BanDuration >= 0 AND
        (BanStarted + BanDuration) > ?)
    OR BanDuration IS NULL
    )");
    s.bind(1, currTime);

    std::vector<BanLocationInfo> out;
    while (s.executeStep()) {
        out.push_back({
            .ip = s.getColumn(0),
            .bouncer = s.getColumn(1),
            .port = s.getColumn(2).isNull() ? std::nullopt : std::optional(s.getColumn(2))
        });
    }
    
    return out;
}

}
