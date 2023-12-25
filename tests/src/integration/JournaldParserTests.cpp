#ifdef INTEGRATION_TESTS
#include "dnf2b/bouncers/NoopBouncer.hpp"
#include "dnf2b/data/ReadStateDB.hpp"
#include "dnf2b/core/Daemon.hpp"
#include "dnf2b/sources/JournalCTLParser.hpp"
#include "dnf2b/util/Clock.hpp"
#include <unistd.h>

#include "nlohmann/json.hpp"
#include <catch2/catch_test_macros.hpp>
#include <systemd/sd-journal.h>
#include <dnf2b/json/Config.hpp>

#include <util/DBWrapper.hpp>

TEST_CASE("Verify buffer integration", "[Integration]") {
    tests::util::DBWrapper wrapper;
    auto start = dnf2b::Clock::now();
    uint64_t startMicrosecs = std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch()).count();

    // TODO: create wrapper 
    auto& db = dnf2b::ReadStateDB::getInstance();
    // Fuck with the database for the parser
    db.store("sshd", "dnf2b_test", startMicrosecs);

    dnf2b::ConfigRoot conf;
    conf.bouncers["noop"] = { 
        {"stfu", true},
    };
    conf.watchers.push_back({
        {"id", "potato"},
        {"process", "dnf2b_test"},
        {"enabled", true},
        {"limit", 0},
        {"parser", "sshd"},
        {"filters", std::vector<std::string>{"sshd-bruteforce"}},
        {"banaction", "noop"}
    });

    std::vector<std::string> messages{
        "error: kex_exchange_identification: Connection closed by remote host",
        "Connection closed by 192.168.0.69 port 4",
    };
    for (auto& message : messages) {
        // TODO: figure out what namespace this yeets messages into
        // Had to drop system only, which feels sketch, though I can't explain why
        REQUIRE(sd_journal_send(
            ("MESSAGE=" + message).c_str(),
            "SYSLOG_IDENTIFIER=dnf2b_test",
            NULL
        ) == 0);
    }

    // The daemon needs to be initialised after the write to make sure it doesn't need to wait
    // a full cycle to read the messages.
    // This isn't really a problem in practice
    dnf2b::Daemon d(conf);
    const auto& pipeline = d.getMessagePipelines();
    REQUIRE(pipeline.size() == 1);
    REQUIRE(pipeline.contains("dnf2b_test"));
    REQUIRE(pipeline.at("dnf2b_test").parser->enableBuffer());


    SECTION("Validate raw reads") {
        {
            dnf2b::JournalCTL throwaway(
                "dnf2b_test",
                startMicrosecs,
                dnf2b::JournalCTL::IDMethod::SYSLOG_IDENTIFIER
            );

            // Dummy read; if this fails, the journal actually failed to get the data
            // If this passes and the next call fails, something else is fucked
            REQUIRE(sd_journal_next(throwaway.ptr()) > 0);
        }

        dnf2b::JournalCTL journal(
            "dnf2b_test",
            startMicrosecs,
            dnf2b::JournalCTL::IDMethod::SYSLOG_IDENTIFIER
        );
        REQUIRE(journal.journal != nullptr);

        size_t count = 0;
        journal.read([&](const auto& message, auto, const auto&, const auto& pid) {
            REQUIRE(pid == std::to_string(getpid()));
            count += 1;
            INFO(message);
        });

        REQUIRE(count == 2);
    }
    SECTION("Validate daemon functionality") {
        size_t count = 0;
        dnf2b::NoopBouncer::_ban = [&](const auto& ip, auto) {
            REQUIRE(ip == "192.168.0.69");
            ++count;
        };

        std::thread t([&]() {
            d.run();
        });
        std::this_thread::sleep_for(10s);

        d.shutdown();

        t.join();
        REQUIRE(t.joinable() == false);
        REQUIRE(count == 1);
    } 

}

#endif
