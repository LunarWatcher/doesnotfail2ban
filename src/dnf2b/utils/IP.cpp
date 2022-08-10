#include "IP.hpp"

namespace dnf2b {

IP::IP(const std::vector<int>& components, int ipRevision, int rangeBits)
        : components(components), rangeBits(rangeBits), ipRevision(ipRevision) {

}

bool in(const IP& other) {
    if (other.ipRevision != ipRevision) return false;
    else if (rangeBits <= 0) {
        // If the range bits are 0 or less, that's equivalent to /0, which
        // can be disregarded.
        // It's a fancy way of indicating that it isn't a range, okay?
        return false; 
    }

    throw "TODO";
}
}
