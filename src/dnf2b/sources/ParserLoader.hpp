#pragma once

#include "dnf2b/sources/Parser.hpp"
namespace dnf2b::ParserLoader {

extern std::shared_ptr<Parser> loadParser(
    const std::string& parserName,
    const std::string& resourceName
);

}
