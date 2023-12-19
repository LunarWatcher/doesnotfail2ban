#include "Parsing.hpp"
#include "spdlog/spdlog.h"
#include <chrono>
#include <stdexcept>
#include <variant>

namespace dnf2b {

long long Parsing::parseConfigToSeconds(std::variant<std::string, long long> source) {
    if (std::holds_alternative<long long>(source)) {
        auto days = std::get<long long>(source);

        if (days <= 0) {
            return -1;
        }

        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::days(days)
        ).count();
    }

    auto str = std::get<std::string>(source);
    // Optimisation
    if (str == "-1") {
        return -1;
    }

    std::size_t pos;
    long long res = std::stoll(str, &pos, 10);

    if (pos >= str.size()) {
        // Malformed string
        spdlog::warn("Interpreting {} as a malformed integer, i.e. parsing as days. If this is not intended, fix  your qualifier. To silence this warning, don't make pure ints strings",
                     str);

        return parseConfigToSeconds(res);
    } else if (res <= 0) {
        return -1;
    }

    auto durationType = str[pos];
    std::chrono::seconds out;
    
    switch (durationType) {
    case 'd':
        out = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::days(res)
        );
        break;
    case 'w':
        out = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::weeks(res)
        );
        break;
    case 'm':
        out = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::months(res)
        );
        break;
    default:
        spdlog::error("{} (from the value {}) is not a valid duration shortener. Must be d (days), w (weeks), or m (months). Unqualified numbers are interpreted as days.",
                      durationType, str);
        throw std::runtime_error("Configuration error");
    }

    return out.count();
}

}
