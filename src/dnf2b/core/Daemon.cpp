#include "Daemon.hpp"
#include "dnf2b/data/ReadStateDB.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/sources/ParserLoader.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <thread>

using namespace std::literals;

namespace dnf2b {


Daemon::Daemon(const ConfigRoot& conf) : conf(conf), man(this->conf) {
    reload();
}

void Daemon::reload() {

    for (auto& watcher : conf.watchers) {
        auto enabled = watcher.value("enabled", true);
        std::string file;
        if (watcher.contains("file")) {
            file = watcher.at("file");
        } else if (watcher.contains("resource")) {
            file = watcher.at("resource");
        } else {
            file = watcher.at("process");
        }

        if (!enabled) {
            spdlog::debug("Skipping watcher for resource {}: Disabled", file);
            continue;
        }
        auto id = watcher.at("id");

        auto& pipeline = messagePipelines[file];
        auto parserName = watcher.at("parser").get<std::string>();

        if (pipeline.parser == nullptr) {
            spdlog::info("Parser not found for {}; initialising...", file);

            pipeline.parser = ParserLoader::loadParser(parserName, file);
        } else if (pipeline.parser->parserName != parserName) {
            spdlog::error("Error: file {} used by watcher ID {} was attempted loaded with parser {}, while it already uses the {} parser");
            throw std::runtime_error("Config error");
        }

        auto logParser = watcher.at("parser");

        auto port = watcher.contains("port") ? std::optional(watcher.at("port").get<uint16_t>()) : std::nullopt;
        auto multiProcessId = watcher.contains("process") ? std::optional(watcher.at("process").get<std::string>()) : std::nullopt;
        auto limit = watcher.value("limit", conf.core.control.maxAttempts);
        auto jsonFilters = watcher.at("filters").get<std::vector<std::string>>();
        auto bouncerName = watcher.at("banaction");

        std::vector<Filter> filters;

        std::transform(
            jsonFilters.begin(),
            jsonFilters.end(),
            std::back_inserter(filters),
            [](const auto& filterPath) {
                return Filter{filterPath};
            }
        );

        auto ptrWatcher = std::make_shared<Watcher>(
            id,
            multiProcessId,
            port,
            limit,
            filters,
            bouncerName
        );

        pipeline.watchers.push_back(ptrWatcher);
    }

    spdlog::info("Watching {} file{} for changes", messagePipelines.size(), messagePipelines.size() != 1 ? "s": "");
}


void Daemon::startUnbanMonitoring() {
    spdlog::info("Unban monitor live. Loading rebans...");
    // Loading rebans is offloaded to another service
    man.loadRebans();
    spdlog::info("Rebans issued");

    while (true) {
        spdlog::debug("Unban monitor woken up");
        man.checkUnbansAndCleanup();

        // Heavily rate limited, because it doesn't need to run faster
        // Close to real-time bans are far more important than close to real-time
        // unbans
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

void Daemon::run() {
    unban = std::thread(&Daemon::startUnbanMonitoring, this);

    spdlog::info("Daemon is live and watching for evil shit");
    std::vector<std::thread> threads;

    for (auto v : messagePipelines) {
        auto _file = v.first;
        auto pipeline = v.second;
        auto thread = std::thread([_file, pipeline, this]() -> void {
            while (true) {
                auto& [parser, watchers] = pipeline;

                auto messages = parser->poll();

                if (messages.size() != 0) {
                    spdlog::debug("{} has new entries", _file);
                    for (auto& watcher : watchers) {
                        decltype(messages) filteredMessages;
                        if (parser->multiprocess && watcher->getProcessID().has_value()) {
                            for (auto& message : messages) {
                                if (message.process == watcher->getProcessID()) {
                                    filteredMessages.push_back(message);
                                }
                            }
                        } else {
                            filteredMessages = messages;
                        }

                        {
                            auto result = watcher->process(filteredMessages);
                            if (result.size() > 0) {
                                man.log(watcher.get(), result);
                            }
                        }
                    }

                } else {
                    ReadStateDB::getInstance().commit();
                    spdlog::debug("{} has nothing new", _file);
                }
                std::this_thread::sleep_for(30s);
            }
        });

        threads.push_back(std::move(thread));
    }

    for (auto& thread : threads) {
        thread.join();
        spdlog::warn("A parser thread has shut down. If you're seeing this without the program being about to shut down (i.e. it's expected that the threads get killed, "
                     "something has gone terribly wrong, and you should be concerned. Consult the logs to see what went sideways.");
    }
}

}
