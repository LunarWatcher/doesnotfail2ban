#pragma once

namespace dnf2b {

class Daemon {
public:
    Context ctx;

    Daemon(const Context& ctx);

    void run();

};

}
