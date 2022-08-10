#include "Context.hpp"

namespace dnf2b {

// TODO: figure out how yaml-cpp error handles LoadFile()
Context::Context(const std::vector<std::string>& arguments) : config(YAML::LoadFile("/etc/dnf2b/config.yaml")), arguments(arguments) {}

}
