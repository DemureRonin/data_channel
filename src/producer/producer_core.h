#pragma once
#include <condition_variable>
#include <queue>
#include <utility>
#include <thread>
#include <atomic>
#include <string>

#include "../common/channel_data.h"
#include "../common/shared_memory_producer.h"

/**
 * @brief Core producer class: reads file, compresses data, writes to SHM
 *
 * Three-thread pipeline:
 * - ReadFromFile: reads raw data → raw_data_queue
 * - HandleCompress: compresses raw data → compressed_queue
 * - HandleWrite: sends compressed packets to SHM
 */
class ProducerCore {
public:
    explicit ProducerCore(std::string input_file, std::string output_file, bool compress)
        : input_file_(std::move(input_file)), output_file_(std::move(output_file)), compress_(compress) {
    }

    /**
     * @brief Run the producer pipeline
     * @return TransferReport with timing and compression statistics
     */
    [[nodiscard]] TransferReport Run();

    /**
     * @brief Build transfer report from collected metrics
     * @return TransferReport containing times, sizes, compression ratio
     */
    [[nodiscard]] TransferReport FormReport() const;

private:
    void HandleReadFromFile();

    size_t SplitIntoPackets(const RawData &raw_data);

    void HandleCompress();

    size_t CompareFiles() const;

    void HandleWrite();

private:
    std::string input_file_;
    std::string output_file_;
    bool compress_;

    SharedMemoryProducer writer_;

    std::queue<RawData> raw_data_queue_;
    std::mutex raw_data_queue_mtx_;
    std::condition_variable raw_data_cv_;
    std::atomic<bool> is_done_reading_{false};

    std::queue<DataPacket> compressed_queue_;
    std::mutex compressed_queue_mtx_;
    std::condition_variable compressed_cv_;
    std::atomic<bool> is_done_compressing_{false};

    std::thread read_thread_;
    std::thread compress_thread_;
    std::thread write_thread_;

    long long read_time_ = 0;
    long long compress_time_ = 0;
    long long write_time_ = 0;

    size_t total_compressed_bytes_ = 0;
    size_t total_bytes_read_ = 0;
    size_t loss_ = 0;
};
