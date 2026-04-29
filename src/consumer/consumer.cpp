#include "consumer_core.h"
#include <iostream>
#include <string>

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: consumer <output_file> [--no-compress]" << std::endl;
        return 1;
    }

    bool compress = true;
    if (argc >= 3 && std::string(argv[2]) == "--no-compress") {
        compress = false;
    }
    {
        ConsumerCore consumer(argv[1], compress);
    }




    return 0;
}
