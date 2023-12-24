#pragma once

#include "dnf2b/filters/Filter.hpp"
#include <vector>
#include <string>
#include <dnf2b/sources/Parser.hpp>
#include <map>

namespace dnf2b {

/**
 * Contains a Message with a missing IP that's waiting for a later
 * message to parse it.
 *
 * This exists in the off chance there's either race conditions between the writing process
 * and the read performed by dnf2b, or other cases where the message containing IPs or other
 * relevant identities is delayed for whatever fucking reason.
 */
struct MessageHold {
    std::vector<Message> msg;
    int failedChecks = 0;
};

// yes, this is basically a pair in disguise, and it's fiiiiiiine
struct IPBuffer {
    std::string ip;
    int passedCycles = 0;
};

/**
 * This class is in charge of buffering messages until they're ready.
 * 
 */
class MessageBuffer {
private:
    std::map<std::string, MessageHold> hold;
    /**
     * Identifier-IP buffer map. Similarly to the message hold, this also has a cycle tracker to yeet
     * inactive processes.
     */
    std::map<std::string, IPBuffer> buff; 

    std::vector<Message> out;
    bool enableCache;
public:
    MessageBuffer(bool enable) : enableCache(enable) {}

    void load(const std::vector<Message>& messages);
    std::map<std::string, std::vector<Message>> forward(Filter& f);

    void registerMessage(const Message& msg);
    void registerAndFree(const Message& msg, std::vector<Message>& out);
    void tryLoadIP(Message& msg);

    void done();

    const decltype(hold)& getHold() { return hold; }
    const decltype(buff)& getBuff() { return buff; }
    const decltype(out)& getOut() { return out; }
};

}
