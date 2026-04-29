#pragma once

#include <array>
#include <string>
#include <vector>


const std::string SEM_EMPTY = "/sem_empty";
const std::string SEM_FULL = "/sem_full";
const std::string SHARED_MEM_NAME = "/data_channel";

constexpr int COMPRESSION_FACTOR = 7;
constexpr int MAX_PACKET_BUFFER_SIZE = 248;
constexpr int MAX_READ_BYTES = 1120;
constexpr int SHM_SIZE = 256;


struct DataPacket {
    std::array<char, MAX_PACKET_BUFFER_SIZE> buffer = std::array<char, MAX_PACKET_BUFFER_SIZE>();
    uint16_t size = 0;
    uint16_t original_count = 0;
    bool is_last = false;
};

struct RawData {
    std::vector<char> buffer = std::vector<char>(MAX_READ_BYTES);
    uint16_t size = 0;
    uint16_t original_count = 0;
    bool is_last = false;
};



