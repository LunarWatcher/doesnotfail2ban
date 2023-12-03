#include "Daemon.hpp"

#include <thread>

using namespace std::literals;

namespace dnf2b {


Daemon::Daemon(const Context& ctx) : ctx(ctx) {}

void Daemon::run() {

    while (true) {


        std::this_thread::sleep_for(30s);
    }
}


}
