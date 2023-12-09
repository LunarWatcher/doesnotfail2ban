#include "Daemon.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/sources/ParserLoader.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <thread>

using namespace std::literals;

namespace dnf2b {


Daemon::Daemon(const Context& ctx) : ctx(ctx), man(ctx.getConfig()) {
    reload();
}

void Daemon::reload() {
    for (auto& watcher : ctx.getConfig().at("watchers")) {
        auto enabled = watcher.value("enabled", true);
        auto file = watcher.at("file");

        if (!enabled) {
            spdlog::debug("Skipping watcher for file {}: Disabled", file);
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
        auto limit = watcher.value("limit", ctx.getMaxAttempts());
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
}


void Daemon::startUnbanMonitoring() {
    // Loading rebans is offloaded to another service
    man.loadRebans();

    while (true) {
        man.checkUnbansAndCleanup();

        // Heavily rate limited, because it doesn't need to run faster
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

void Daemon::run() {
    unban = std::thread(&Daemon::startUnbanMonitoring, this);

    while (true) {
        for (auto& [_file, pipeline] : messagePipelines) {
            auto& [parser, watchers] = pipeline;

            auto messages = parser->poll();

            if (messages.size() != 0) {
                for (auto& watcher : watchers) {
                    auto result = watcher->process(messages);
                    if (result.size() > 0) {
                        man.log(watcher.get(), result);
                    }
                }

            } else {
                spdlog::debug("{} has nothing new", _file);
            }
        }

        std::this_thread::sleep_for(30s);
    }
}

}
