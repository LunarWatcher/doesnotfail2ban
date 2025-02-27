#include "Parser.hpp"

#include "dnf2b/static/Constants.hpp"
#include "dnf2b/util/PCRE.hpp"
#include "pcre2.h"
#include "spdlog/spdlog.h"

#include <chrono>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <string>
#include <iostream>

namespace dnf2b {

Parser::Parser(const std::string& parserName, const nlohmann::json& config, const std::string& resourceName)
    :
        config(config),
        parserName(parserName),
        resourceName(resourceName),
        multiprocess(config.value("multiprocess", false))
{

    if (config.contains("pattern")) {
        auto p = config.at("pattern");
        if (p.is_object()) {
            //config.at("pattern").at("full");
            pattern.emplace(
                Pattern(
                    p.at("full").get<std::string>(),
                    PCRE2_CASELESS | PCRE2_UTF
                )
            );
            timeFormat = p.value("time", "");
        } else {
            pattern.emplace(
                Pattern(
                    p.get<std::string>(),
                    PCRE2_CASELESS | PCRE2_UTF
                )
            );
        }
    }

    if (config.contains("ipFallbackSearch")) {
        auto obj = config.at("ipFallbackSearch");
        auto pattern = obj.at("regex");

        this->fallbackSearch.emplace(IPFallbackSearch {
            .p = pattern.get<std::string>()
        });
    }
}

std::optional<Message> Parser::parse(const std::string& line) {
    if (!pattern) {
        spdlog::error("Parser invoked parse() without pattern.full defined.");
        throw 255;
    }

    PCREMatcher m(*pattern, line);

    if (!m.next()) {
        return std::nullopt;
    }


    // If a timestamp isn't part of the parsed output, assume current timestamp
    auto now = std::chrono::system_clock::now();
    if (auto timeString = m.get("Time"); timeString.has_value()) {
        // This is messy. Stay with me
        // First of all, fuck time in C++. It's garbage. HowardHinnant/date, which is now
        // C++20, is still crap. 
        // Ergo, this code is also crap. Would've been so much easier if people didn't
        // make time so absurdly complicated, but here we are.
        // Fuck time.
        //
        // This crap isn't initialized with shit:
        //     std::tm tm = {};
        // And it isn't easily populated, and because I'm not particularly steady in the
        // C API, emergency solution time
        // This ensures we get the current time.
        std::time_t currTimeMillis = std::time(nullptr);
        // Which we dump into a localtime struct, because
        // we have 0 contextual info

        auto raw = std::localtime(&currTimeMillis);
        tm tm = *raw;

        // we now have a tm struct prepopulated with all the necessary
        // dates for the current time
        // What we then do is shove the parsed time on top of the predefined values:
        std::stringstream ss(timeString.value());
        ss >> std::get_time(&tm, this->timeFormat.c_str());
        // The struct isn't wiped, fortunately, so any data in there not overridden, such as
        // where the time specifiers are missing, is preserved

        // The constructor for message then takes care of making a time from the struct, which
        // is then parsed into a timepoint or whatever the fuck it's called.
        //
        // I'm not sure I'm fully clear on why this works, but I assume it's down to more time fuckery.
        // Part of some other problems came from the year missing, which presumably means going
        // back to a point where DST didn't exist (DST is such fucking trash, let's just get rid
        // of it already), which might mean different timezones?
        //
        // I don't know, I didn't check the timezone information in the tm struct when dumping.
        // Idfk, this crap works, deal with it.
        //
        // I'm not sure if std::localtime when dumping the time again actually preserves information though,
        // BUT since gmtime results in 20:17:44, I'm inclined to say it does.
        //
        // Fuck knows what's in the chrono timepoint though. Looking forward to getting back to
        // my debugger and poking around the internals of chrono.
        //
        // TODO: deserialize the config, so lookups aren't necessary

        now = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    }

    auto message = m.get("Msg");
    auto host = m.get("Host");
    auto ip = m.get("IP");

    if (!message.has_value()) {
        spdlog::error("{} has a misconfigured parser format. Msg is missing", this->parserName);
        return std::nullopt;
    }

    return Message {
        .entryDate = now,
        .host = host.value_or(""),
        .message = *message,
        .ip = ip.value_or(""),
    };
}

// TODO: reconsider this function
std::vector<Message> Parser::filterMessages(const std::string&, const std::vector<Message>& messages) {
    // If the input isn't multiprocess, it's guaranteed correct
    // Note that some inputs are multiprocess, but aren't declared multiprocess. 
    //
    // Nginx is one good example of this. It does some logging of the services, users, and return codes, but not which port stuff
    // is forwarded to. This means further filtering isn't appropriate before it's passed to the watcher.
    // 
    if (!this->config.at("multiprocess").get<bool>()) {
        return messages;
    }
    std::vector<Message> out;

    

    return out;
}

std::optional<std::string> IPFallbackSearch::search(const Message& m) {
    PCREMatcher matcher(p, m.message);

    if (matcher.next()) {
        // If a match is found, return the IP group
        // If it doesn't exist, .get returns std::nullopt,
        // so no further validation is needed
        return matcher.get("IP");
    }
    return std::nullopt;
}

}
