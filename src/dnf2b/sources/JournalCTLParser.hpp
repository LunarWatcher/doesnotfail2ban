#pragma once

#include "dnf2b/sources/Parser.hpp"

#include <systemd/sd-journal.h>

namespace dnf2b {

class JournalCTL {
public:
    sd_journal* journal;

    JournalCTL(const std::string& syslogID, uint64_t since);
    ~JournalCTL();

    void read(std::function<void(const std::string& message, uint64_t microsecTime, const std::string& hostname)> callback);
};

class JournalCTLParser : public Parser {
private:
    uint64_t lastAccessedDate;

public:
    JournalCTLParser(const std::string& parserName, const nlohmann::json& config, const std::string& fileName) : Parser(parserName, config, fileName), lastAccessedDate(0) {}
    ~JournalCTLParser();

    std::vector<Message> poll() override;

    void close(const std::string&) override {}

};

}
