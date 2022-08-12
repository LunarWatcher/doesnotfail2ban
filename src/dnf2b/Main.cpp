#include<iostream>
#include<unistd.h>

#include "dnf2b/ui/CLI.hpp" 

int main(int argc, const char* argv[]) {
    // Non-root users have a uid != 0, which we want to disallow.
    // This does also hide the help command behind root, but it's a necessary evil
    if (getuid() != 0) {
        std::cerr << "Must be root to run the program." << std::endl;
        return -1;
    }
    try {
        dnf2b::CLI::parse(argc, argv);
    } catch (const int& x) {
        // I'm abusing int throwing as an overcomplicated way to return the value,
        // because why not? It works, and it's the only way to do this in a system where
        // whether the errors warrant a restart is up to the specific error.
        return x;
    }
}
