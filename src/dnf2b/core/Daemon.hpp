#pragma once

#include "dnf2b/data/BanManager.hpp"
#include "dnf2b/data/MessageBuffer.hpp"
#include "dnf2b/json/Config.hpp"
#include "dnf2b/sources/FileParser.hpp"
#include "dnf2b/util/Clock.hpp"
#include "dnf2b/watcher/Watcher.hpp"
#include <chrono>
#include <condition_variable>
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>

namespace dnf2b {

struct MessagePipeline {
    std::shared_ptr<Parser> parser;
    std::vector<std::shared_ptr<Watcher>> watchers;
    std::shared_ptr<MessageBuffer> buff;
};

class Daemon {
private:
    std::map<std::string, MessagePipeline> messagePipelines;
    std::thread unban;
    ConfigRoot conf;
    BanManager man;

    std::shared_mutex cvLock;
    std::condition_variable_any runFlag;
    bool isRunning = true;

    /**
     * Utility wrapper around the runFlag condition variable for interruptable sleeping
     */
    void wait(std::chrono::seconds duration);

    void startUnbanMonitoring();
public:

    Daemon(const ConfigRoot& conf);
    ~Daemon() {
        shutdown();
    }

    void reload();

    void run();
    void shutdown() {
        isRunning = false;
        runFlag.notify_all();

        if (unban.joinable()) {
            unban.join();
        }
    }

    const decltype(messagePipelines)& getMessagePipelines() { return messagePipelines; }

};

}
