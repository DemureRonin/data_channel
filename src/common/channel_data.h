#pragma once

/**
 * @file channel_data.h
 * @brief Shared structures for interprocess communication
 */
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <array>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

const std::string SEM_EMPTY = "/sem_empty";
const std::string SEM_FULL = "/sem_full";
const std::string SEM_DONE = "/sem_done";
const std::string SHARED_MEM_NAME = "/data_channel";

constexpr int COMPRESSION_FACTOR = 7;
constexpr int MAX_PACKET_BUFFER_SIZE = 248;
constexpr int MAX_READ_BYTES = 1120;
constexpr int SHM_SIZE = 256;

/**
 * @brief Fixed-size packet for transmission via shared memory
 */
struct DataPacket {
    std::array<char, MAX_PACKET_BUFFER_SIZE> buffer = std::array<char, MAX_PACKET_BUFFER_SIZE>();
    uint16_t size = 0;
    uint16_t original_count = 0;
    bool is_last = false;
};

/**
 * @brief Raw data block read from input file
 */
struct RawData {
    std::vector<char> buffer = std::vector<char>(MAX_READ_BYTES);
    uint16_t size = 0;
    uint16_t original_count = 0;
    bool is_last = false;
};

/**
 * @brief Performance report for Producer
 */
struct TransferReport {
    size_t read_from_file_time;
    size_t compress_time;
    size_t write_to_shm_time;
    size_t total_time;

    size_t bytes_read_from_file;
    size_t bytes_sent_to_shm;
    size_t compression_ratio;
    size_t space_saved;
    size_t loss;
};

/**
 * @brief Force close all semaphores on interrupt
 */
[[maybe_unused]] static void signal_handler(int sig) {
    shm_unlink(SHARED_MEM_NAME.c_str());
    sem_unlink(SEM_EMPTY.c_str());
    sem_unlink(SEM_FULL.c_str());
    sem_unlink(SEM_DONE.c_str());
    std::exit(sig);
}

/**
 * @brief Print transfer report to console
 */
[[maybe_unused]] static void PrintReport(const TransferReport &report) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "         TRANSFER REPORT                " << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "\n[ TIME STATISTICS ]" << std::endl;
    std::cout << "  Read from file:     " << report.read_from_file_time << " ms" << std::endl;
    std::cout << "  Compress data:      " << report.compress_time << " ms" << std::endl;
    std::cout << "  Write to SHM:       " << report.write_to_shm_time << " ms" << std::endl;
    std::cout << "  ----------------------------------------" << std::endl;
    std::cout << "  TOTAL TIME:         " << report.total_time << " ms" << std::endl;

    std::cout << "\n[ SIZE STATISTICS ]" << std::endl;
    std::cout << "  Bytes read:         " << report.bytes_read_from_file << " bytes";
    if (report.bytes_read_from_file > 1024 * 1024) {
        std::cout << " (" << std::fixed << std::setprecision(2)
                << static_cast<double>(report.bytes_read_from_file) / (1024 * 1024) << " MB)";
    }
    std::cout << std::endl;

    std::cout << "  Bytes sent to SHM:  " << report.bytes_sent_to_shm << " bytes";
    if (report.bytes_sent_to_shm > 1024 * 1024) {
        std::cout << " (" << std::fixed << std::setprecision(2)
                << static_cast<double>(report.bytes_sent_to_shm) / (1024 * 1024) << " MB)";
    }
    std::cout << std::endl;

    std::cout << "\n[ COMPRESSION STATISTICS ]" << std::endl;
    std::cout << "  Compression ratio:  " << std::fixed << std::setprecision(2)
            << report.compression_ratio << "x" << std::endl;
    std::cout << "  Space saved:        " << std::fixed << std::setprecision(2)
            << report.space_saved << "%" << std::endl;


    std::cout << "\n[ LOSS STATISTICS ]" << std::endl;
    std::cout << "  Data loss:          " << std::fixed << std::setprecision(4)
            << report.loss << "%" << std::endl;


    std::cout << "\n========================================" << std::endl;
}
