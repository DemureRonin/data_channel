#pragma once
#include <condition_variable>

#include <queue>
#include <utility>

#include "../common/channel_data.h"

#include "../common/shared_memory_producer.h"
#include "../common/zfp_codec.h"


class ProducerCore {
public:
    explicit ProducerCore(std::string file_name, bool compress) : file_name_(std::move(file_name)), compress_(compress) {
        read_thread_ = std::thread(&ProducerCore::ReadFromFile, this);
        write_thread_ = std::thread(&ProducerCore::WriteToSharedMemory, this);
        compress_thread_ = std::thread(&ProducerCore::HandleCompress, this);
    }

    ~ProducerCore() {
        if (read_thread_.joinable()) read_thread_.join();
        if (write_thread_.joinable()) write_thread_.join();
        if (compress_thread_.joinable()) compress_thread_.join();
    }

    void ReadFromFile();

    size_t SplitIntoPackets(const RawData &raw_data);

    void WriteToSharedMemory();

    void HandleCompress();

    void PrintTotalTime();

private:
    std::string file_name_;

    SharedMemoryProducer writer_;

    std::queue<RawData> raw_data_queue_;
    std::queue<DataPacket> packet_queue_;

    std::mutex raw_data_queue_mtx_;
    std::mutex packet_queue_mtx_;

    std::condition_variable is_done_reading_block_;
    std::atomic<bool> is_done_reading_;

    std::condition_variable is_done_compressing_block_;
    std::atomic<bool> is_done_compressing_;

    std::thread read_thread_;
    std::thread compress_thread_;
    std::thread write_thread_;

    long long read_time_ = 0;
    long long compress_time_ = 0;
    long long write_time_ = 0;
    size_t total_compressed_bytes_ = 0;

    bool compress_;
};
