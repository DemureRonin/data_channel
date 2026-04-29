#include <fstream>
#include <iostream>
#include <string>
#include "producer_core.h"

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: producer <input_file> [--no-compress]" << std::endl;
        return 1;
    }

    bool compress = true;
    if (argc >= 3 && std::string(argv[2]) == "--no-compress") {
        compress = false;
    }

    {
        ProducerCore core(argv[1], compress);
        auto report = core.Run();
        PrintReport(report);
    }

    return 0;
}
