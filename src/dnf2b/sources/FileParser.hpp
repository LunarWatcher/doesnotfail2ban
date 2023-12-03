#pragma once

#include <chrono>
#include <map>
#include <string>
#include <memory>
#include <utility>
#include <regex>
#include <fstream>

#include "Parser.hpp"

namespace dnf2b {

class Context;
class FileParser : public Parser {
private:
    std::map<std::string, size_t> lastAccessedByte;
public:
    FileParser(const std::string& parserName) : Parser(parserName) {}

    std::vector<Message> poll(const std::string& resourceName) override;
    //std::optional<Message> parse(Context& ctx, const std::string& line) override;
    
    void close(const std::string& resourceName) override;

    std::string checkHealth() override;
};

}
