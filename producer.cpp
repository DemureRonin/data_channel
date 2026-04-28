#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "i_compressor.h"

std::queue<DataPacket> q;
std::mutex mtx;
void ProcessRead(std::string file_name) {
    std::ifstream file(file_name, std::ios::binary);
    std::array<char, 64 * sizeof(float)> buffer;
    if (file.read(buffer.data(), buffer.size())) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push({.buffer = buffer});
    }
}
void ProcessWrite() {

    while (!q.empty()) {
        std::lock_guard<std::mutex> lock(mtx);
        DataPacket data = q.front();
        q.pop();
        for (int i = 0; i < 63;     i++) {
            std::cout << reinterpret_cast<float*>(data.buffer[i]) << std::endl;
        }

    }
}

int main( int argc, const char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: producer <filename>" << std::endl;
        return 1;
    }

    std::thread reader(ProcessRead, argv[1]);
    std::thread writer(ProcessWrite);

    reader.join();
    writer.join();
    return 0;
}
