#include "MockHelper.hpp"
#include "dnf2b/static/Constants.hpp"

#include <fstream>
#include <stdexcept>

namespace tests {

void Mock::modifyConfig(const nlohmann::json &conf) {
    std::ofstream f(dnf2b::Constants::DNF2B_ROOT / "config.local.json");
    if (!f) {
        throw std::runtime_error("Failed to open stream to modify mock config");
    }
    f << conf.dump(4) << std::endl;
}

}
