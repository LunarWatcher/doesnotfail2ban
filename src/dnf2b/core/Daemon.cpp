#include "Daemon.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <thread>

using namespace std::literals;

namespace dnf2b {


Daemon::Daemon(const Context& ctx) : ctx(ctx) {
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

        auto port = watcher.at("port").get<uint16_t>();
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

void Daemon::run() {

    while (true) {
        for (auto& [_file, pipeline] : messagePipelines) {
            auto& [parser, watchers] = pipeline;

            auto messages = parser->poll();

            if (messages.size() != 0) {
                for (auto& watcher : watchers) {

                }

            } else {
                spdlog::debug("{} has nothing new", _file);
            }
        }

        std::this_thread::sleep_for(30s);
    }
}

}
