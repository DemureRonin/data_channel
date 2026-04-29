#include "consumer_core.h"

int main() {
    try {
        {
            ConsumerCore consumer("out.bin");
        }

        {
            ConsumerCore consumer1("out.bin");
        }
    } catch (const std::exception &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
