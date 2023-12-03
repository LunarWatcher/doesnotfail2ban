#include <iostream>

#include "dnf2b/ui/CLI.hpp" 

int main(int argc, const char* argv[]) {
    try {
        return dnf2b::CLI::parse(argc, argv);
    } catch (const int& x) {
        // I'm abusing int throwing as an overcomplicated way to return the value,
        // because why not? It works, and it's the only way to do this in a system where
        // whether the errors warrant a restart is up to the specific error.
        //
        // Future me here: What the actual fuck was past me thinking?
        return x;
    }
}
