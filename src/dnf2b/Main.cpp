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
    dnf2b::CLI::parse(argc, argv);
}
