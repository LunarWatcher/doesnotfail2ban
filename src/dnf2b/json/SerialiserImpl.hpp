#include "nlohmann/adl_serializer.hpp"
#include <nlohmann/json.hpp>

namespace nlohmann {
template <class T>
struct adl_serializer<std::optional<T>> {
    static void to_json(nlohmann::json &j, const std::optional<T>& value) {
        if (value.has_value()) {
            j = *value;
        }
    }
    static void from_json(const nlohmann::json &j, std::optional<T>& value) {
        if (!j.empty() && !j.is_null()) {
            value = j;
        }
    }
};
}
