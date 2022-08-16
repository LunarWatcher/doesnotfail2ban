#pragma once

#include "dnf2b/infrastructure/HealthCheck.hpp"

namespace dn2fb {

class Context;
class Communicator : public HealthCheck {
public:
    Communicator();

    virtual bool shouldAlert(Context& ctx) = 0;
    virtual void doAlert(Context& ctx) = 0;

    virtual std::string checkHealth() = 0;
};

}
