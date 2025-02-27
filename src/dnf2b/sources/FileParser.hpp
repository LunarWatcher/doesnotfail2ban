#pragma once

#include <string>

#include "Parser.hpp"

namespace dnf2b {

class Context;
class FileParser : public Parser {
private:
    size_t lastAccessedByte;

public:
    FileParser(const std::string& parserName, const nlohmann::json& config, const std::string& fileName) : Parser(parserName, config, fileName), lastAccessedByte(0) {}
    virtual ~FileParser() = default;

    std::vector<Message> poll() override;
    //std::optional<Message> parse(Context& ctx, const std::string& line) override;
    
    void close(const std::string& resourceName) override;

};

}
