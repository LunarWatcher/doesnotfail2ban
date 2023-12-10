#pragma once

#include "dnf2b/sources/Parser.hpp"

#include <systemd/sd-journal.h>

namespace dnf2b {

class JournalCTL {
private:
    std::string parseValue(const void* raw, size_t length);
public:
    enum class IDMethod {
        SYSLOG_IDENTIFIER,
        SYSTEMD_UNIT
    };
    sd_journal* journal;

    JournalCTL(const std::string& syslogID, uint64_t since, IDMethod method);
    ~JournalCTL();

    void read(std::function<void(const std::string& message, uint64_t microsecTime, const std::string& hostname)> callback);

    static std::string getField(IDMethod method);
};

class JournalCTLParser : public Parser {
private:
    uint64_t lastAccessedDate;
    JournalCTL::IDMethod method;

public:
    JournalCTLParser(const std::string& parserName, const nlohmann::json& config, const std::string& fileName);

    virtual ~JournalCTLParser() = default;

    std::vector<Message> poll() override;

    void close(const std::string&) override {}

};

}
