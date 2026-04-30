#include <fstream>
#include <iostream>
#include <string>
#include "producer_core.h"
#include <csignal>


int main(int argc, const char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    if (argc < 3) {
        std::cout << "Usage: producer <input_file> <output_file> [--no-compress]" << std::endl;
        return 1;
    }

    bool compress = true;
    if (argc >= 4) {
        std::string flag = argv[3];
        if (flag == "--no-compress") {
            compress = false;
        } else {
            std::cout << "Unknown option: " << flag << std::endl;
            std::cout << "Usage: producer <input_file> <output_file> [--no-compress]" << std::endl;
            return 1;
        }
    }

    ProducerCore core(argv[1], argv[2], compress);
    auto report = core.Run();
    PrintReport(report);

    return 0;
}
