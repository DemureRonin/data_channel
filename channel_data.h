#pragma once
#include <array>

struct DataPacket {
    std::array<char, 64 * sizeof(float)> buffer;
};
