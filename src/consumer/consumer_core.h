#pragma once
#include <condition_variable>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <utility>

#include "../common/shared_memory_consumer.h"
#include "../common/zfp_codec.h"

class ConsumerCore {
public:
    ConsumerCore(std::string output_file, bool compress)
        : output_file_(std::move(output_file)), compress_(compress) {
        read_thread_ = std::thread(&ConsumerCore::ReadFromSharedMemory, this);
        decompress_thread_ = std::thread(&ConsumerCore::HandleDecompress, this);
        write_thread_ = std::thread(&ConsumerCore::WriteToFile, this);
    }

    ~ConsumerCore() {
        if (read_thread_.joinable()) read_thread_.join();
        if (decompress_thread_.joinable()) decompress_thread_.join();
        if (write_thread_.joinable()) write_thread_.join();
    }

private:
    void ReadFromSharedMemory();
    void HandleDecompress();
    void WriteToFile();

private:
    std::string output_file_;
    bool compress_;

    SharedMemoryConsumer reader_;

    std::queue<DataPacket> packet_queue_;
    std::queue<RawData> raw_data_queue_;

    std::mutex packet_queue_mtx_;
    std::mutex raw_data_queue_mtx_;

    std::condition_variable packet_cv_;
    std::condition_variable raw_data_cv_;

    std::thread read_thread_;
    std::thread decompress_thread_;
    std::thread write_thread_;
};