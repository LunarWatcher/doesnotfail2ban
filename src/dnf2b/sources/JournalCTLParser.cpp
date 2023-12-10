#include "JournalCTLParser.hpp"
#include "spdlog/spdlog.h"
#include <cstring>
#include <stdexcept>
#include <string>
#include <systemd/sd-journal.h>
#include <algorithm>

namespace dnf2b {

JournalCTL::JournalCTL(const std::string& syslogID, uint64_t since, IDMethod method) {
    if (int r = sd_journal_open(&journal, SD_JOURNAL_SYSTEM | SD_JOURNAL_LOCAL_ONLY); r < 0) {
        spdlog::error("Failed to open journald journal: {}", strerror(-r));
        return;
    }

    if (since > 0) {
        if (sd_journal_seek_realtime_usec(journal, since * 1'000'000) < 0) {
            spdlog::error("Seek failed");
            throw std::runtime_error("Journald failed");
        }
    }
    

    if (sd_journal_add_match(journal, fmt::format("{}={}", getField(method), syslogID).c_str(), 0) < 0) {
        spdlog::error("Failed to add _SYSTEMD_UNIT={} as a match", syslogID);
        throw std::runtime_error("Failed to subscribe to systemd unit");
    }
}

JournalCTL::~JournalCTL() {
    if (journal != nullptr) {
        sd_journal_close(journal);
    }
}

void JournalCTL::read(std::function<void(const std::string& message, uint64_t microsecTime, const std::string& hostname)> callback) {
    if (!callback) {
        spdlog::warn("Programmer error: callback to journalctl::read is null");
        return;
    }

    SD_JOURNAL_FOREACH(journal) {
        const void *data;
        size_t length;

        std::string message = "";
        uint64_t messageDate = 0;
        std::string hostname = "";

        if (sd_journal_get_data(journal, "__REALTIME_TIMESTAMP", &data, &length) >= 0
            || sd_journal_get_data(journal, "_SOURCE_REALTIME_TIMESTAMP", &data, &length) >= 0) {
            std::string rawMessageDate = parseValue(data, length);
            messageDate = std::stoull(rawMessageDate);
        } else {
            spdlog::error("Critical: failed to load timestamp from either __REALTIME_TIMESTAMP or _SOURCE_REALTIME_TIMESTAMP.");
            throw std::runtime_error("Failed to load date");
        }

        if (sd_journal_get_data(journal, "MESSAGE", &data, &length) >= 0) {
            message = parseValue(data, length);
        }

        if (sd_journal_get_data(journal, "_HOSTNAME", &data, &length) >= 0) {
            hostname = parseValue(data, length);
        }

        callback(message, messageDate, hostname);
    }
}

std::string JournalCTL::parseValue(const void* raw, size_t length) {
    // No clue if this is reliable
    std::string message((const char*) raw, length);

    auto res = message.substr(message.find("=") + 1);
    return res;
}

std::string JournalCTL::getField(IDMethod method) {
    switch (method) {
    case IDMethod::SYSLOG_IDENTIFIER:
        return "SYSLOG_IDENTIFIER";
    case IDMethod::SYSTEMD_UNIT:
        return "_SYSTEMD_UNIT";
    }
}

JournalCTLParser::JournalCTLParser(const std::string& parserName, const nlohmann::json& config, const std::string& fileName) : Parser(parserName, config, fileName), lastAccessedDate(0) {
    auto rawMethod = config.value("idMethod", "syslog");
    if (rawMethod == "syslog") {
        method = JournalCTL::IDMethod::SYSLOG_IDENTIFIER;
    } else if (rawMethod == "systemd_unit") {
        method = JournalCTL::IDMethod::SYSTEMD_UNIT;
    } else {
        spdlog::error("Config error: {} is not a valid idMethod.", rawMethod);
        throw std::runtime_error("Config error");
    }
}

std::vector<Message> JournalCTLParser::poll() {
    JournalCTL j = {
        resourceName,
        lastAccessedDate,
        method
    };

    if (j.journal == nullptr) {
        spdlog::error("Journal has nullptr");
        return {};
    }

    std::vector<Message> out;

    j.read([&](const auto& message, auto time, const auto& host) -> void {
        auto normalisedTimeSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::microseconds(time));

        out.push_back(Message {
            .entryDate = std::chrono::system_clock::time_point(normalisedTimeSecs),
            .host = host,
            .message = message,
        });
        this->lastAccessedDate = std::max(this->lastAccessedDate, time);
    });
    return out;
}

}
