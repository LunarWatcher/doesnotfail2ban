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
    std::map<std::string, std::pair<std::shared_ptr<std::ifstream>, size_t>> streams;
public:
    FileParser(const std::string& parserName) : Parser(parserName) {}

    std::vector<Message> poll(Context& ctx, const nlohmann::json& config) override;
    //std::optional<Message> parse(Context& ctx, const std::string& line) override;
    
    void close(const std::string& resourceName) override;

    std::string checkHealth() override;
};

}
