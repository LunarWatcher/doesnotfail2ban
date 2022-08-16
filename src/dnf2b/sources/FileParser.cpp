#include "FileParser.hpp"

#include <filesystem>

#include "dnf2b/core/Context.hpp"

namespace dnf2b {

// TODO: test
std::vector<Message> FileParser::poll(Context& ctx, const nlohmann::json& config) {
    std::vector<Message> messages;

    const std::string& resourceName = config.at("file").get<std::string>();

    std::shared_ptr<std::ifstream> ptr = nullptr;
    size_t size;

    if (streams.find(resourceName) != streams.end()) {
        ptr = std::make_shared<std::ifstream>(resourceName);
        size = std::filesystem::file_size(resourceName);
        streams[resourceName] = {ptr, size};
    } else {
        auto& pair = streams.at(resourceName);
        ptr = pair.first;
        size = pair.second;

        auto newSize = std::filesystem::file_size(resourceName);

        if (newSize == size) {
            // This doesn't properly account for files with a fixed size, but can't think of any files like that
            // that don't border binary-like files
            return {};
        } else if (newSize < size) {
            ptr->open(resourceName);
        }
    }

    ptr->clear();

    // I fucked up while removing the `parse` method; I roped it into filters, which is bad.
    // Bring it back later, kthx
    //std::string cache;
    //while (std::getline(*ptr, cache)) {
        //auto message = parse(ctx, cache);
        //if (message) {
            //messages.push_back(*message);
        //}
    //}

    return messages;
}


void FileParser::close(const std::string& resourceName) {
    // TODO
}

std::string FileParser::checkHealth() {
    std::string health = "FileParser[" + parserName + "]\n";

    // Check the status of resources or whatever

    return health;
}

}
