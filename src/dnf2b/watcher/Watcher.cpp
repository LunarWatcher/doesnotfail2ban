#include "Watcher.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "spdlog/spdlog.h"

#include <iostream>

namespace dnf2b {

Watcher::Watcher(
    const std::string& id,
    std::optional<std::string> multiProcessID,
    std::optional<uint16_t> port,
    int limit,
    const std::vector<Filter>& filters,
    const std::string& bouncer
)
    : id(id), multiProcessID(multiProcessID), port(port), limit(limit), filters(filters), bouncer(bouncer) {

}

std::map<std::string, std::vector<Message>> Watcher::process(
    const std::vector<Message>& messages,
    std::shared_ptr<MessageBuffer> buff
) {
    buff->load(messages);
    std::map<std::string, std::vector<Message>> out;

    for (auto& filter : filters) {
        auto intermediateRes = buff->forward(filter);
        for (auto& [k, v] : intermediateRes) {
            if (out.contains(k)) {
                auto& vec = out.at(k);
                vec.insert(vec.end(), v.begin(), v.end());
            } else {
                out[k] = v;
            }
        }
    }

    // Required to clean up the buffer's `load`ed memory, and clean up the stale caches
    buff->done();

    return out;
}



}
