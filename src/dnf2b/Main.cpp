#include<iostream>

#include <unistd.h>

int main() {
    if (!getuid()) {
        std::cerr << "Must be root to run the program." << std::endl;
        return -1;
    }
    std::cout << "Hello, World!" << std::endl;
}
