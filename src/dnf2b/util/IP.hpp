#pragma once

#include <string>
#include <variant>

namespace dnf2b {

struct IPv4 {

};
struct IPv6 {

};

class IP {
private:
    std::variant<IPv4, IPv6> underlying;
    std::string rawIP;
public:
    IP(const std::string& ip);
};

}
