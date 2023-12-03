#pragma once

#include "Context.hpp"
#include "dnf2b/sources/FileParser.hpp"
#include <map>
#include <string>

namespace dnf2b {

class Daemon {
private:
    std::map<std::string, FileParser> parsers;

public:
    Context ctx;

    Daemon(const Context& ctx);

    void reload();

    void run();

};

}
