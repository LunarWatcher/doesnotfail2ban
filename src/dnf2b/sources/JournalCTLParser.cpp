#include "JournalCTLParser.hpp"
#include "dnf2b/util/PCRE.hpp"
#include "spdlog/spdlog.h"
#include <cstring>
#include <stdexcept>
#include <string>
#include <systemd/sd-journal.h>
#include <algorithm>
#include <dnf2b/data/ReadStateDB.hpp>

namespace dnf2b {

JournalCTL::JournalCTL(const std::string& syslogID, uint64_t since, IDMethod method) {
    if (int r = sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY); r < 0) {
        spdlog::error("Failed to open journald journal: {}", strerror(-r));
        return;
    }

    // TODO: add filtering for additional fields, at least for test purposes. Might be worth moving such filter
    // calls out of the constructor?
    if (sd_journal_add_match(journal, fmt::format("{}={}", getField(method), syslogID).c_str(), 0) < 0) {
        spdlog::error("Failed to add field type as a match", syslogID);
        throw std::runtime_error("Failed to subscribe to systemd unit");
    }

    if (since > 0) {
        if (sd_journal_seek_realtime_usec(journal, since) < 0) {
            spdlog::error("Seek failed");
            throw std::runtime_error("Journald failed");
        }
    }
}

JournalCTL::~JournalCTL() {
    if (journal != nullptr) {
        sd_journal_close(journal);
    }
}

void JournalCTL::read(ReadCallback callback) {
    if (!callback) {
        spdlog::warn("Programmer error: callback to journalctl::read is null");
        return;
    }

    // Poll for updates with a timeout of 0
    // Note that hasRead is present to ensure this isn't blocked until there's a new update
    // That would prevent anything caught on the first sweep from being processed until later
    if (hasRead && sd_journal_wait(journal, 0) != SD_JOURNAL_APPEND) {
        // If nothing new, return
        return;
    }
    hasRead = true;

    // https://github.com/systemd/systemd/issues/1752
    //SD_JOURNAL_FOREACH(journal) {
    while (sd_journal_next(journal) > 0) {
        const void *data;
        size_t length;

        std::string message = "";
        uint64_t messageDate = 0;
        std::string hostname = "";
        std::string pid = "";

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

        if (sd_journal_get_data(journal, "_PID", &data, &length) >= 0
            || sd_journal_get_data(journal, "SYSLOG_PID", &data, &length) >= 0) {
            pid = parseValue(data, length);
        }

        callback(message, messageDate, hostname, pid);
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

    // Don't have unreachable yet
    [[unlikely]]
    throw std::runtime_error("Unreachable");
}

JournalCTLParser::JournalCTLParser(const std::string& parserName, const nlohmann::json& config, const std::string& fileName)
        : Parser(parserName, config, fileName), lastAccessedDate(0) {
    auto rawMethod = config.value("idMethod", "syslog");
    if (rawMethod == "syslog") {
        method = JournalCTL::IDMethod::SYSLOG_IDENTIFIER;
    } else if (rawMethod == "systemd_unit") {
        method = JournalCTL::IDMethod::SYSTEMD_UNIT;
    } else {
        spdlog::error("Config error: {} is not a valid idMethod.", rawMethod);
        throw std::runtime_error("Config error");
    }
    auto state = ReadStateDB::getInstance().read(parserName, resourceName);
    if (state.has_value()) {
        // Can nlohmann/json even store unsigned 64 bit ints?
        // Tbf, as long as it stores an int64_t, it should be fine.
        // It just means the uint risks overflowing to a negative number when converted to a
        // signed int64_t, but they'll then overflow again back to more or less the correct
        // positive value.
        // It'll be fine
        lastAccessedDate = state.value().get<uint64_t>();
        spdlog::debug("Loaded lastAccessedDate = {} from ReadStateDB", lastAccessedDate);
    }

    j = std::make_shared<JournalCTL>(
        resourceName,
        lastAccessedDate,
        method
    );
}

std::vector<Message> JournalCTLParser::poll() {

    if (j->journal == nullptr) {
        spdlog::error("Journal has nullptr");
        return {};
    }

    std::vector<Message> out;

    j->read([&](auto message, auto time, const auto& host, const auto& pid) -> void {
        // I'm sure this won't come back to haunt me.
        // I'm definitely sure this won't have any problems with DST, and that journalctl timestamps are, in fact, monotonous.
        // Tbf, DST is probably going to fuck over the FileParser when I get around to beefing it up
        // with time checks
        if (time < lastAccessedDate) {
            return;
        }
        auto normalisedTimeSecs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::microseconds(time));
        std::string ip = "";
        if (this->pattern.has_value()) {
            PCREMatcher m(*pattern, message);

            if (!m.next()) {
                spdlog::debug("Failed to pattern-match {} against {}'s regex. Using raw message instead",
                    message, this->parserName);
                return;
            } else {
                message = m.get("Msg").value();
                ip = m.get("IP").value_or("");
            }
        }

        auto msg = Message {
            .entryDate = std::chrono::system_clock::time_point(normalisedTimeSecs),
            .host = host,
            .message = message,
            .ip = ip,
            .process = pid
        };
        if (fallbackSearch && msg.ip.size() == 0) {
            msg.ip = fallbackSearch->search(msg)
                .value_or(msg.ip);
            //spdlog::debug("Fallback search found IP: {}", msg.ip);
        }

        out.push_back(std::move(msg));

        this->lastAccessedDate = std::max(this->lastAccessedDate, time);
    });

    ReadStateDB::getInstance().store(this->parserName, this->resourceName, lastAccessedDate);

    return out;
}

}
