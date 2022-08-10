#include "IP.hpp"

namespace dnf2b {

IP::IP(const std::vector<int>& components, int ipRevision)
        : components(components), ipRevision(ipRevision) {

}

bool in(const IP& other) {
    if (other.ipRevision != ipRevision) return false;

    throw "TODO";
}
}
