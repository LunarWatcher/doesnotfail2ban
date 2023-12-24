#include "MessageBuffer.hpp"
#include "dnf2b/filters/Filter.hpp"
#include <map>

#include <arpa/inet.h>

namespace dnf2b {

void MessageBuffer::load(const std::vector<Message>& in) {
    for (auto message : in) {
        if (message.ip.size() != 0) {
            // If the IP isn't blank, set the cache and free the held messages
            // from the cache, if fany.
            registerAndFree(message, out);
        } else {
            // If the IP doesn't exist, try to load it from a buffer before it's processed
            tryLoadIP(message);
        }
        out.push_back(message);
    }
}

std::map<std::string, std::vector<Message>> MessageBuffer::forward(Filter& f) {
    std::map<std::string, std::vector<Message>> caught;
    for (auto& message : out) {
        auto res = f.checkMessage(message);
        if (!res) {
            // No match, all good
            continue;
        }

        if (res->error) {
            if (!enableCache) {
                spdlog::error("Fallback search disabled and IP missing from pattern: skipping. You probably have a bad filter");
                continue;
            }
            if (buff.contains(message.process)) {
                // If this runs, the IP has been added by a previously executed message, or
                // during the load process. This should help ensure nothing is accidentally held for more than one cycle
                res->ip = buff.at(message.process).ip;
            } else {
                hold[message.process].msg.push_back(message);
                continue;
            }
        }

        if (res->ip.size()) {
            // TODO: migrate to asio::ip

            // IPv6 is guaranteed to contain :, IPv4 is not. We use this to identify which to use
            int domain = res->ip.find(":") == std::string::npos ? AF_INET : AF_INET6;

            auto str = res->ip.c_str();
            // Unused
            unsigned char buf[sizeof(struct in6_addr)];
            int isValid = inet_pton(domain, str, buf);

            if (isValid == 0) {
                spdlog::error("{} failed to parse as {} (see inet_pton(3)). This is probably a programmer error", res->ip, domain);
                continue;
            } else if (isValid == -1) {
                spdlog::error("Programmer error: Domain isn't valid (found {}, must be AF_INET or AF_INET6)", domain);
                throw std::runtime_error("Fatal programmer error");
            }

            spdlog::info("{} failed and will be logged.", res->ip);
            // store the IP, as the message itself is part of the returned content
            message.ip = res->ip;
            // Push the message into the IP buffer
            registerMessage(message);

            caught[message.ip].push_back(message);
        }
    }

    return caught;
}

void MessageBuffer::done() {
    out.clear();
    if (!enableCache) {
        return;
    }
    // AFAIK, there's no good way to avoid two iterations here.
    // And I mean one iteration to increment and one iteration to remove,
    // not that there's one iteration for hold and one for buff.
    //
    // With cbegin() and cend(), it's possible to iterate and loop, but that's
    // just std::erase_if(). begin() and end() apparently can't be modified while
    // iterating, which forces one loop to modify, and one to remove.
    //
    // There's probably a better way to do this. Clangd didn't immediately complain
    // when I removed the const and added a ++ in front of the number, but that
    // feels like it's UB, so didn't bother looking more into it.
    for (auto& [_, held] : hold) {
        held.failedChecks += 1;
    }
    for (auto& [_, ipCache] : buff) {
        ipCache.passedCycles += 1;
    }

    std::erase_if(
        hold,
        [](const auto& e) -> bool {
            return e.second.failedChecks >= 5;
        }
    );
    std::erase_if(
        buff,
        [](const auto& e) -> bool {
            return e.second.passedCycles >= 5;
        }
    );
}

void MessageBuffer::tryLoadIP(Message& msg) {
    if (!enableCache) {
        return;
    }
    if (buff.contains(msg.process)) {
        msg.ip = buff.at(msg.process).ip;
    }
}

void MessageBuffer::registerMessage(const Message& msg) {
    if (!enableCache) {
        return;
    }
    // Any new observations in an existing process
    // will fully update the entry, which I'm not sure is efficient,
    // but the main purpose is to reset the cycle. This avoids
    // two lookups, but I'm not sure if that makes any meaningful
    // difference
    buff[msg.process] = {
        msg.ip,
        0
    };
}

void MessageBuffer::registerAndFree(const Message& msg, std::vector<Message>& out) {
    if (!enableCache) {
        return;
    }
    registerMessage(msg);

    if (auto it = hold.find(msg.process); it != hold.end()) {

        for (auto& badMessage : it->second.msg) {
            badMessage.ip = msg.ip;
            out.push_back(badMessage);
        }

        hold.erase(it);
    }
}

}
