#pragma once

#include "dnf2b/core/Context.hpp"
#include "nlohmann/json.hpp"

#include <filesystem>
#include <functional>

namespace dn2fb {

namespace Wizard {

extern nlohmann::json detectSSH(Context& ctx);

extern void launchWizard();

static inline std::vector<std::function<nlohmann::json()>> funcs {
    detectSSH
};

}

}
