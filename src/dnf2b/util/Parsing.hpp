#pragma once

#include <string>
#include <variant>
namespace dnf2b::Parsing {

extern long long parseConfigToSeconds(std::variant<std::string, long long> source);

}
