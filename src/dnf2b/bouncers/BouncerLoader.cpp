#include "BouncerLoader.hpp"
#include "dnf2b/bouncers/IPTableBouncer.hpp"
#include "dnf2b/bouncers/NoopBouncer.hpp"
#include "spdlog/spdlog.h"

namespace dnf2b {

std::shared_ptr<Bouncer> BouncerLoader::loadBouncer(const std::string &bouncerName, const nlohmann::json &config) {
    if (bouncerName == "iptables") {
        return std::make_shared<IPTableBouncer>(config);
    } else if (bouncerName == "noop") {
        if (!config.contains("stfu")) {
            spdlog::warn("Noop bouncer requested. Note that this bouncer name is short for \"no operation\", and does nothing. If you're not doing this in a test context, you're probably doing something stupid right now");
        }
        return std::make_shared<NoopBouncer>();
    } else {
        spdlog::error("{} is not a valid bouncer name", bouncerName);
        throw std::runtime_error("Invalid bouncer");
    }
}

}
