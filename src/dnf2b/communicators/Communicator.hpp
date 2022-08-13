#pragma once

#include "dnf2b/infrastructure/HealthCheck.hpp"

namespace dn2fb {

class Communicator : public HealthCheck {
protected:
    Context* ctx;

public:
    Communicator(Context* ctx);

    virtual bool shouldAlert() = 0;
    virtual void doAlert() = 0;

    virtual std::string checkHealth() = 0;
};

}
