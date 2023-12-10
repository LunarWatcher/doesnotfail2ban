#include "JournalCTLParser.hpp"
#include "spdlog/spdlog.h"
#include <cstring>
#include <stdexcept>
#include <systemd/sd-journal.h>

namespace dnf2b {

JournalCTL::JournalCTL(const std::string& syslogID, uint64_t since) {
    if (int r = sd_journal_open(&journal, SD_JOURNAL_SYSTEM | SD_JOURNAL_LOCAL_ONLY); r < 0) {
        spdlog::error("Failed to open journald journal: {}", strerror(-r));
        return;
    }

    if (sd_journal_seek_realtime_usec(journal, since * 1'000'000) < 0) {
        spdlog::error("Seek failed");
        throw std::runtime_error("Journald failed");
    }
    

    if (sd_journal_add_match(journal, fmt::format("_SYSTEMD_UNIT={}", syslogID).c_str(), 0) < 0) {
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

        std::string message;
        uint64_t messageDate;
        std::string hostname;

        if (sd_journal_get_data(journal, "__REALTIME_TIMESTAMP", &data, &length) >= 0) {
            time_t timestamp = *(uint64_t *)data;
        }

        if (sd_journal_get_data(journal, "MESSAGE", &data, &length) >= 0) {
            message = std::string(static_cast<const char*>(data), length);
        }

        if (sd_journal_get_data(journal, "_HOSTNAME", &data, &length) >= 0) {
            hostname = std::string(static_cast<const char*>(data), length);
        }

        callback(message, messageDate, hostname);
    }
}

JournalCTLParser::~JournalCTLParser() {
    
}

std::vector<Message> JournalCTLParser::poll() {
    JournalCTL j = {
        resourceName,
        lastAccessedDate
    };

    std::vector<Message> out;

    j.read([&out](const auto& message, auto time, const auto& host) -> void {
        auto normalisedTimeSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::microseconds(time));

        out.push_back(Message {
            .entryDate = std::chrono::system_clock::time_point(normalisedTimeSecs),
            .host = host,
            .message = message,
        });
    });
    return out;
}

}
