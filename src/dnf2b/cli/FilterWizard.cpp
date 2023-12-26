#include "FilterWizard.hpp"
#include "dnf2b/filters/Filter.hpp"
#include "dnf2b/static/Constants.hpp"
#include "dnf2b/util/Colour.hpp"
#include "dnf2b/util/PCRE.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <unistd.h>

namespace dnf2b {

void CLI::filterWizard(bool isInsensitive) {
    nlohmann::json j {
        {"filters", std::vector<std::string>()},
        {"insensitive", isInsensitive}
    };

    auto& storedFilters = j.at("filters");
    std::vector<std::pair<std::string, bool>> messages;

    std::cout << "Welcome to the filter wizard." << std::endl;

    if (geteuid() != 0) {
        std::cout << Colour::background(160)
            << Colour::foreground(231)
            << "WARNING:"
            << Colour::clear
            << " If you don't run the wizard as sudo, the filter cannot be automatically written to a file."
            << " You'll instead get the output as JSON printed in the terminal, and you'll then have to manually"
            << " copy it to a file to use it. If you're fine with this, you can disregard this warning."
            << "\n\n";
    }

    std::cout << "Enter messages to match. Note that this will not auto-generate the regex, but is meant to aid with debugging. "
        << "Note: Do not include messages scaffolding. For example, if your log line looks like:\n"
        << "\t[core] Aug 17 21:22:23: some log message\n"
        << "Only enter \"some log message\" (without quotes).\n"
        << "Enter an empty message to stop." << std::endl;

    while (true) {
        std::string line;
        std::cout << "Message: ";
        std::getline(std::cin, line);

        if (line == "EOF" || line.size() == 0) {
            break;
        }

        messages.push_back({line, false});
    }

    if (messages.size() == 0) {
        std::cout << "Note: Because you didn't provide any messages, debug functionality is disabled" << std::endl;
    }
    std::cout << "\n";

    std::cout << "You'll now be asked to enter a regex pattern. You'll be shown a preview of matches, "
        << "and the pattern will not be added to the final filter unless you approve it.\n"
        << "Also note that  you don't need to escape anything. If you type \\S, that's read as the literal \\S and "
        << "not an escape code. The output escaping is handled automatically by dnf2b.\n";

    if (messages.size()) {
        std::cout << "Colour legend:\n";

        std::cout << Colour::foreground(160) << "\tNot matched by any patterns in the current filter" << Colour::clear << std::endl;
        std::cout << Colour::foreground(214) << "\tMatched by a different pattern in the current filter" << Colour::clear << std::endl;
        std::cout << Colour::foreground(40) << "\tMatched by the current filter" << Colour::clear << std::endl;
        std::cout << "As a reminder, a single filter contains multiple patterns. It's encouraged not to match all the messages with a single pattern, unless they're very similar." << std::endl;
        
    }
    std::cout << std::endl;

    while (true) {
        std::string line;
        std::cout << "Enter an empty pattern to stop\n";
        // TODO: standardise the special key interface so I don't have to do this shit
        // Or yeet the key, it doesn't really make sense
        std::cout << "To match an IP, use \"(?<IP>\\S+)\" or similar. The special ${dnf2b.ip} key is not supported by this wizard\n";
        std::cout << "Pattern: ";
        std::getline(std::cin, line);
        if (line == "") {
            break;
        }
        Pattern p(line);
        if (messages.size()) {
            std::cout << "Matches: " << std::endl;
            for (auto& [message, matched] : messages) {
                PCREMatcher m(p, message);
                if (m.next()) {
                    matched = true;
                    std::cout << "\t" << Colour::foreground(40) << message << Colour::clear << std::endl;
                } else {

                    std::cout << Colour::foreground(matched ? 214 : 160)
                        << "\t" << message << Colour::clear << std::endl;
                }
            }

        } else {
            std::cout << "Cannot preview matches as you didn't provide any sample messages." << std::endl;
        }

        while (true) {
            std::cout << "Keep this pattern? (y/n) ";
            std::string r;
            std::getline(std::cin, r);

            if (r == "y") {
                storedFilters.push_back(line);
                break;
            } else if (r == "n") {
                break;
            }
        }
    }

    std::cout << "What would you like to do with the filter?\n";

    while (true) {
        std::string command;
        std::cout << "Discard (d)/Accept (a)/Output to file (f): ";
        std::getline(std::cin, command);

        if (command == "d") {
            std::cout << "Discarded" << std::endl;
            return;
        } else if (command == "a" || (command == "f" && geteuid() != 0)) {
            std::cout << "Filter json:" << std::endl;
            std::cout << j.dump(4) << std::endl;
            return;
        } else if (command == "f" && geteuid() == 0) {
            while (true) {
                std::string filterName;
                std::cout << "Filter name: ";
                std::getline(std::cin, filterName);

                if (filterName != "") {
                    auto path = Constants::DNF2B_ROOT / "filters" / (filterName + ".json");
                    if (std::filesystem::exists(path)) {
                        std::cout << path.string() << " already exists. Try a different name." << std::endl;
                        continue;
                    }
                    std::ofstream ss(path);

                    ss << j.dump(4) << std::endl;
                    break;
                } else {
                    std::cout << j.dump(4) << std::endl;
                    break;
                }
            }
            return;
        }
    }
}

}
