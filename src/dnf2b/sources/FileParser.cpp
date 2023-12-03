#include "FileParser.hpp"

#include <filesystem>

#include "dnf2b/core/Context.hpp"
#include <spdlog/spdlog.h>

namespace dnf2b {

std::vector<Message> FileParser::poll(const std::string& resourceName) {
    std::vector<Message> messages;


    auto size = lastAccessedByte.contains(resourceName) ? lastAccessedByte.at(resourceName) : 0;
    std::ifstream stream(resourceName);

    // Seek to the last opened position
    stream.seekg(size, std::ios::cur);

    if (stream.fail() || stream.eof()) {
        // If we fail or hit eof and size == 0, the  file has been wiped and is empty. Ignore
        if (size == 0) {
            return {};
        }

        // Otherwise, seek back to 0. The logfile has been wiped since last time
        stream.seekg(0, std::ios::cur);
    }

    stream.seekg(0, std::ios::end);
    auto newPos = stream.tellg();
    if (newPos == -1) {
        spdlog::error("{} returned -1.", resourceName);
    }
    lastAccessedByte[resourceName] = newPos;

    return messages;
}


void FileParser::close(const std::string&) {}

std::string FileParser::checkHealth() {
    std::string health = "FileParser[" + parserName + "]\n";

    // Check the status of resources or whatever

    return health;
}

}
