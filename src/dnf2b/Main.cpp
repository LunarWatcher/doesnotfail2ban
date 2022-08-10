#include<iostream>
#include<unistd.h>

#include "dnf2b/ui/CLI.hpp" 

int main(int argc, const char* argv[]) {
    if (!getuid()) {
        std::cerr << "Must be root to run the program." << std::endl;
        return -1;
    }
    dnf2b::CLI::parse(argc, argv);
}
