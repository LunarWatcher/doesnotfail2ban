#include "Watcher.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <iostream>

namespace dnf2b {

Watcher::Watcher(const std::string& id, std::optional<std::string> multiProcessID, uint16_t port, int limit, const std::vector<Filter>& filters, const std::string& groupId)
    : id(id), multiProcessID(multiProcessID), port(port), limit(limit), filters(filters), groupId(groupId) {

}

std::map<std::string, int> Watcher::process(const std::vector<Message>& messages) {
    std::map<std::string, int> resultIps;
    for (const auto& message : messages) {
        for (auto& filter : filters) {
            auto res = filter.checkMessage(message);
            if (res.has_value()) {
                // IPv6 is guaranteed to contain :, IPv4 is not. We use this to identify which to use
                int domain = res->ip.find(":") == std::string::npos ? AF_INET : AF_INET6;

                auto str = res->ip.c_str();
                // Unused
                unsigned char buf[sizeof(struct in6_addr)];
                int isValid = inet_pton(domain, str, buf);

                if (isValid == 0) {
                    spdlog::error("{} failed to parse as {} (see inet_pton(3))", res->ip, domain);
                } else if (isValid == -1) {
                    spdlog::error("Programmer error: Domain isn't valid (found {}, must be AF_INET or AF_INET6)", domain);
                } else {
                    spdlog::info("{} failed and will be logged.", res->ip);
                    resultIps[res->ip] += 1;
                }
            }
        }
    }

    return resultIps;
}



}
