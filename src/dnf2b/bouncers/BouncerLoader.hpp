#pragma once

#include "dnf2b/bouncers/Bouncer.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <memory>

namespace dnf2b::BouncerLoader {

extern std::shared_ptr<Bouncer> loadBouncer(const std::string& bouncerName, const nlohmann::json& config);

}
