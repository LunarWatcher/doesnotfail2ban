#pragma once

#include "spdlog/spdlog.h"
#include <map>
#include <optional>
#include <pcre2.h>
#include <string>

namespace dnf2b {

class Pattern;
class PCREMatcher {
private:
    std::string input;
    pcre2_match_data* matchData;

    /**
     * Note: managed by a Pattern sourced elsewhere.
     * Do not free this pointer.
     */
    pcre2_code* pattern;
    size_t* ovector = nullptr;

    int matchIndex;
    size_t count = 0;

public:
    PCREMatcher(pcre2_code* pattern, const std::string& input);
    ~PCREMatcher() {
        if (matchData) {
            pcre2_match_data_free(matchData);
        }
    }

    PCREMatcher(PCREMatcher&& m)
        : input(m.input),
            matchData(std::move(m.matchData)), 
            matchIndex(m.matchIndex), 
            count(m.count) {}

    std::string get(size_t group);
    std::string get(const std::string& group);

    bool next();
    int getMatchGroups() { return count; }
};

class Pattern {
private:
    pcre2_code* pattern;
public:

    Pattern(const std::string& patternStr, int options = 0);
    ~Pattern() {
        if (pattern) {
            pcre2_code_free(pattern);
        }
    }

    operator pcre2_code*() {
        return pattern;
    }
};

}
