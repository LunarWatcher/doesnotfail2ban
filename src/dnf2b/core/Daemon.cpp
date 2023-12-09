#include "Daemon.hpp"
#include "dnf2b/filters/Filter.hpp"
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

        auto pipeline = messagePipelines[file];

        if (pipeline.parser == nullptr) {
            spdlog::info("Parser not found for {}; initialising...", file);
        }

        auto logParser = watcher.at("parser");

        auto port = watcher.contains("port") ? std::optional(watcher.at("port").get<uint16_t>()) : std::nullopt;
        auto limit = watcher.value("limit", ctx.getMaxAttempts());
        auto jsonFilters = watcher.at("filters").get<std::vector<std::string>>();

        auto group = watcher.value("banGroup", "global");

        std::vector<Filter> filters;

        std::transform(
            jsonFilters.begin(),
            jsonFilters.end(),
            std::back_inserter(filters),
            [](const auto& filterPath) {
                return Filter{filterPath};
            }
        );
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
