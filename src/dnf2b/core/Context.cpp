#include "Config.hpp"

namespace dnf2b {

// TODO: figure out how yaml-cpp error handles LoadFile()
Context::Context() : config(YAML::LoadFile("/etc/dnf2b/config.yaml")) {}

}
