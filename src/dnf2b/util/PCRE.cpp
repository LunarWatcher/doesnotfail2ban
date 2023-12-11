#include "PCRE.hpp"
#include "pcre2.h"
#include <stdexcept>
#include <string>

namespace dnf2b {
Pattern::Pattern(const std::string& patternStr, int options) {
    int errorcode;
    size_t errorOffset;

    pattern = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(patternStr.c_str()),
        PCRE2_ZERO_TERMINATED,
        options,
        &errorcode,
        &errorOffset,
        nullptr
    );

    if (!pattern) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        // There is no fucking way this doesn't segfault
        spdlog::error("Regex compilation of \"{}\" failed at {}: {}", 
            patternStr,
            errorOffset,
            (const char*) buffer);
        throw std::runtime_error("Invalid pattern");
    }

}

PCREMatcher::PCREMatcher(pcre2_code* pattern, const std::string& input) : input(input), pattern(pattern) {
    matchData = pcre2_match_data_create_from_pattern(pattern, nullptr);
}

std::optional<std::string> PCREMatcher::get(size_t group) {
    // >=?
    if (group >= count) {
        return std::nullopt;
    }

    auto start = ovector[2 * group];
    auto length = ovector[2 * group + 1] - start;
    // This is probably inefficient as fuck
    return input.substr(start, length);
}

std::optional<std::string> PCREMatcher::get(const std::string& group) {
    if (count <= 1) {
        return std::nullopt;
    }
    auto groupIdx = pcre2_substring_number_from_name(pattern, reinterpret_cast<PCRE2_SPTR>(group.c_str()));
    if (groupIdx >= 0) {
        return get(groupIdx);
    }
    return std::nullopt;
}

bool PCREMatcher::next() {
    int rc = pcre2_match(
        pattern,
        reinterpret_cast<PCRE2_SPTR>(input.c_str()),
        input.length(),
        ovector == nullptr ? 0 : ovector[1],
        0,
        matchData,
        nullptr
    );
    if (rc <= 0) {
        return false;
    }
    count = rc;
    ovector = pcre2_get_ovector_pointer(matchData);

    return true;
}

}
