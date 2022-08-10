#pragma once

namespace dnf2b {

class IP {
private:
    std::vector<int> components;
    int rangeBits;
    
    // Generally IPv4 or IPv6, but generalizing this because I can't be arsed
    // to make an enum, and it's fine. It's not rocket  science.
    int ipRevision;
public:

    IP(const std::vector<int>& components, int ipRevision, int rangeBits = -1);

    // Primarily used for IP ranges
    // Do I even need this?
    bool in(const IP& other);

};

}
