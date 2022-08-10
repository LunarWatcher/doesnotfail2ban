#pragma once

#include <string>
#include <vector>
#include <functional>

namespace dnf2b {

namespace CLI {

extern std::map<std::string, std::function<void(Config&)>> commands;

extern void parse(int argc, const char* argv[]);

}

}
