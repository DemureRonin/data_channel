#include "consumer_core.h"
#include <iostream>

int main() {
    try {
        {
            ConsumerCore consumer("out.bin", false);
        }

        {
            ConsumerCore consumer1("out.bin", true);
        }
    } catch (const std::exception &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
