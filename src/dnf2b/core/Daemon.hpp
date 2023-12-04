#pragma once

#include "Context.hpp"
#include "dnf2b/sources/FileParser.hpp"
#include "dnf2b/watcher/Watcher.hpp"
#include <map>
#include <string>

namespace dnf2b {

struct MessagePipeline {
    std::shared_ptr<FileParser> parser;
    std::vector<std::shared_ptr<Watcher>> watchers;

};

class Daemon {
private:
    std::map<std::string, MessagePipeline> messagePipelines;

public:
    Context ctx;

    Daemon(const Context& ctx);

    void reload();

    void run();

};

}
