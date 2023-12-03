#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "dnf2b/core/Context.hpp"

namespace dnf2b {

namespace CLI {

extern std::map<std::string, std::function<void(Context&)>> commands;

extern int parse(int argc, const char* argv[]);

}

}
