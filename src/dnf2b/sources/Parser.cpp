#include "Parser.hpp"

#include "dnf2b/core/Context.hpp"
#include "dnf2b/static/Constants.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>

namespace dnf2b {

Parser::Parser(const std::string& parserName, const nlohmann::json& config, const std::string& resourceName)
    :
        config(config),
        parserName(parserName),
        resourceName(resourceName),
        multiprocess(config.value("multiprocess", false))
{}

std::optional<Message> Parser::parse(const std::string& line) {
    std::smatch match;
    auto& pattern = config.at("pattern");

    // TODO: sort out flags
    // TODO: figure out what the fuck past me meant by flags in this context
    auto regex = std::regex{pattern["full"].get<std::string>()};

    auto timePattern = pattern["time"].get<std::string>();

    auto& groups = pattern["groups"];

    if (!groups.contains("time")) {
        throw std::runtime_error("Required field \"time\" missing from " + parserName);
    } else if (!groups.contains("message")) {
        throw std::runtime_error("Required field \"message\" missing from " + parserName);
    } else if (!config.contains("multiprocess")) {
        throw std::runtime_error("Required field \"multiprocess\" missing from " + parserName);
    } else if (config["multiprocess"].get<bool>() && !groups.contains("process")) {
        throw std::runtime_error("Required field \"process\" in multiprocess parser missing from " + parserName);
    }

    if (std::regex_match(line, match, regex)) {

        auto timeString = match[groups["time"].get<int>() + 1].str();

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
        std::stringstream ss(timeString);
        ss >> std::get_time(&tm, timePattern.c_str());
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
        return Message {
            // entryDate
            std::chrono::system_clock::from_time_t(std::mktime(&tm)),
            // process
            config["multiprocess"] ? match[groups["process"].get<int>() + 1].str() : "",
            // host
            groups.contains("host") ? match[groups["host"].get<int>() + 1].str() : "",
            // message
            match[groups["message"].get<int>() + 1].str(),
            // IP
            groups.contains("ip") ? match[groups["ip"].get<int>() + 1].str() : ""
        };

    }

    return std::nullopt;
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

}
