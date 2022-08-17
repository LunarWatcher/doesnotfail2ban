#pragma once

#include <exception>

namespace dnf2b {

namespace except {

class GenericException : public std::runtime_error {
public:
    /**
     * The return value for the main() function.
     *
     * Note that any try-catch statements intercepting this class of exceptions
     * MUST rethrow the exception of `responseCode == 255`.
     *
     * `responseCode` set to 255 is fatal and reserved for unrecoverable errors, such as
     * severe, irreparable config errors, errors that should never occur in practice,
     * or broken core components being detected. These should cause the service to halt,
     * which requires user intervention.
     *
     * The exact type of intervention required is not defined by the response code.
     * The fix may range from simply editing some config, to a bug in dnf2b itself that
     * requires code changes.
     *
     * Equivalently, this means that only critical errors that cannot be fixed with a restart
     * should be given 255. All other errors should give value in the range <0, 255>. The value
     * does not have to be unique overall, but if they're known issues, a function that throws
     * multiple of these exceptions for different reasons with different fixes should use different
     * values, and these should be documented as a part of a comment for ease-of-use for end users.
     *
     * Any other response codes can be caught and managed locally.
     */
    const int responseCode;

    GenericException(const std::string& what, int responseCode) : std::runtime_error(what), responseCode(responseCode) {}
};

}

}
