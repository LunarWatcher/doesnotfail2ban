#include "Config.hpp"
#include "dnf2b/util/Parsing.hpp"

namespace dnf2b {



void from_json(const nlohmann::json& j, ControlConfig& o) {
    if (j.contains("maxAttempts")) {
        j.at("maxAttempts").get_to(o.maxAttempts);
    }
    if (j.contains("banIncrement")) {
        j.at("banIncrement").get_to(o.banIncrement);
    }
    if (j.contains("banPeriod")) {
        if (j.at("banPeriod").is_number_integer()) {
            o.banPeriod = Parsing::parseConfigToSeconds(j.at("banPeriod").get<long long>());
        } else {
            o.banPeriod = Parsing::parseConfigToSeconds(j.at("banPeriod").get<std::string>());
        }
    }
    if (j.contains("forgetAfter")) {
        if (j.at("forgetAfter").is_number_integer()) {
            o.forgetAfter = Parsing::parseConfigToSeconds(j.at("forgetAfter").get<long long>());
        } else {
            o.forgetAfter = Parsing::parseConfigToSeconds(j.at("forgetAfter").get<std::string>());
        }
    }
}

void to_json(nlohmann::json& j, const ControlConfig& o) {
    j["maxAttempts"] = o.maxAttempts;
    j["banIncrement"] = o.banIncrement;
    j["banPeriod"] = o.banPeriod;
    j["forgetAfter"] = o.forgetAfter;
}

}
