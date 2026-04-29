#include <fstream>
#include <iostream>
#include "producer_core.h"


int main(int argc, const char *argv[]) {
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: producer <filename>" << std::endl;
        return 1;
    }

    {
        /*ProducerCore core1(argv[1], false);
        auto report = core1.Run();
        PrintReport(report);*/
    }
    {
        ProducerCore core2(argv[1], true);
        auto report = core2.Run();
        PrintReport(report);
    }


    return 0;
}
