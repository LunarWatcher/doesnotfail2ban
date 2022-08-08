#pragma once

#include <string>

namespace dnf2b {

class Bouncer {
public:
    Bouncer();

    virtual std::string toString() = 0;
};

}
