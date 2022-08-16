#include "Daemon.hpp"

#include <thread>

using namespace std::literals;

namespace dnf2b {


Daemon::Daemon(const Context& ctx) : ctx(ctx) {}

void Daemon::run() {
    ctx.start();

    while (true) {
        // general workflow; poll, which also takes care of bouncers
        ctx.poll();
        // TODO: check stats
        // TODO: Dispatch alerts if necessary
        // TODO: check unbans

        std::this_thread::sleep_for(30s);
    }
}


}
