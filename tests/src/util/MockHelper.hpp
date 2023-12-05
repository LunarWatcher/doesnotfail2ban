#pragma once

#include <nlohmann/json.hpp>

namespace tests::Mock {

extern void modifyConfig(const nlohmann::json& conf);

}
