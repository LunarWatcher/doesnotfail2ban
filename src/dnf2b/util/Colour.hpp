#pragma once

#include <cstdint>
#include <ostream>
namespace dnf2b::Colour {

namespace _detail {
inline void _8bit(std::ostream& o, uint8_t colour, int mode) {
    o << "\033[" << mode << ";5;" << static_cast<int>(colour) << "m";
}
}

// This feels dirty as fuck
struct foreground {
    uint8_t colour;

    foreground(uint8_t colour) : colour(colour) {}

    friend auto& operator<<(std::ostream& o, const foreground& f) {
        _detail::_8bit(o, f.colour, 38);
        //o << "Debug: " << f.colour << "; ";
        return o;
    }
};

struct background {
    uint8_t colour;

    background(uint8_t colour) : colour(colour) {}

    friend auto& operator<<(std::ostream& o, const background& b) {
        _detail::_8bit(o, b.colour, 48);
        return o;
    }
};

inline std::ostream& clear(std::ostream& o) {
    return o << "\033[0m";
}

}
