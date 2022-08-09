#pragma once

namespace dnf2b {

class Reader {
public:

    ~Reader() = default;

    void read(const std::string& location);
};

}
