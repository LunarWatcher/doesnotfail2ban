#pragma once


namespace dn2fb {

class Context;
class Communicator {
public:
    Communicator();

    virtual bool shouldAlert(Context& ctx) = 0;
    virtual void doAlert(Context& ctx) = 0;

};

}
