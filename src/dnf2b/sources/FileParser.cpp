#include "FileParser.hpp"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <iostream>

namespace dnf2b {

std::vector<Message> FileParser::poll() {
    std::vector<Message> messages;
    // TODO: This has a bug with file recreation.
    // If, by pure chance, the file is recreated, and is larger than it was
    // prior to the last iteration, this will fail.
    // Should probably cross-compare with some other timestamp for good measure,
    // but this likely means diving into log parsing or maybe some low-level linux
    // stuff for identifiers
    //
    // Problem for future me, though. This is an edge-case, not a blocker

    std::ifstream stream(resourceName);

    // Seek to the last opened position
    stream.seekg(lastAccessedByte, std::ios::cur);

    if (stream.fail() || stream.eof()) {
        // If we fail or hit eof and size == 0, the  file has been wiped and is empty. Ignore
        if (lastAccessedByte == 0) {
            return {};
        }

        lastAccessedByte = 0;
        // Otherwise, seek back to 0. The logfile has been wiped since last time
        stream.seekg(0, std::ios::cur);
    }

    std::string line;
    while (getline(stream, line)) {
        auto mess = parse(line);
        if (mess.has_value()) {
            messages.push_back(*mess);
        } else {
            spdlog::debug("Pattern-matching failed: {}", line);
        }
    }

    // Clear the eof flag
    stream.clear();
    // Search to end to get position
    stream.seekg(0, std::ios::end);
    auto newPos = stream.tellg();
    if (newPos == -1) {
        // Not sure what could trigger this
        spdlog::error("{} returned -1.", resourceName);
        return {};
    }
    lastAccessedByte = newPos;

    return messages;
}


void FileParser::close(const std::string&) {}

}
