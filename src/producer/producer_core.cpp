#include "producer_core.h"
#include "../common/logger.h"
#include <fstream>
#include <chrono>
#include <iostream>

void ProducerCore::ReadFromFile() {
    auto start_time = std::chrono::steady_clock::now();
    std::ifstream file(file_name_, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("failed to open file: " + file_name_);

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t bytes_read_total = 0;

    while (true) {
        RawData raw_data;
        raw_data.buffer.resize(MAX_READ_BYTES);
        file.read(raw_data.buffer.data(), MAX_READ_BYTES);
        size_t bytes_read = file.gcount();

        if (bytes_read == 0) break;

        bytes_read_total += bytes_read;
        raw_data.size = bytes_read;
        raw_data.is_last = (bytes_read_total == file_size);
        raw_data.original_count = bytes_read / sizeof(float);

        {
            std::lock_guard lock(raw_data_queue_mtx_);
            raw_data_queue_.push(raw_data);
        }
        is_done_reading_block_.notify_one();

        if (raw_data.is_last) break;
    }

    is_done_reading_ = true;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    read_time_ = duration.count();
}

size_t ProducerCore::SplitIntoPackets(const RawData &raw_data) {
    size_t total_bytes = 0;
    size_t bytes_remaining = raw_data.size;
    size_t offset = 0;

    while (bytes_remaining > 0) {
        DataPacket packet;
        size_t chunk_size = std::min(bytes_remaining, static_cast<size_t>(MAX_PACKET_BUFFER_SIZE));

        memcpy(packet.buffer.data(), raw_data.buffer.data() + offset, chunk_size);
        packet.size = chunk_size;
        packet.original_count = chunk_size / sizeof(float);
        packet.is_last = (bytes_remaining <= MAX_PACKET_BUFFER_SIZE) && raw_data.is_last;

        {
            std::lock_guard packet_lock(packet_queue_mtx_);
            packet_queue_.push(packet);
        }

        total_bytes += chunk_size;
        offset += chunk_size;
        bytes_remaining -= chunk_size;
    }

    return total_bytes;
}

void ProducerCore::HandleCompress() {
    auto start_time = std::chrono::steady_clock::now();
    size_t total_compressed = 0;

    while (true) {
        std::unique_lock<std::mutex> raw_data_lock(raw_data_queue_mtx_);
        is_done_reading_block_.wait(raw_data_lock, [this]() {
            return !raw_data_queue_.empty() || is_done_reading_;
        });

        if (raw_data_queue_.empty() && is_done_reading_) break;

        auto raw_data = raw_data_queue_.front();
        raw_data_queue_.pop();
        raw_data_lock.unlock();

        if (compress_) {
            auto compressed = ZfpCodec::Compress(raw_data);
            total_compressed += compressed.size;
            {
                std::lock_guard packet_lock(packet_queue_mtx_);
                packet_queue_.push(compressed);
            }
        } else {
            total_compressed += SplitIntoPackets(raw_data);
        }

        is_done_compressing_block_.notify_one();
    }

    is_done_compressing_ = true;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    compress_time_ = duration.count();
    total_compressed_bytes_ = total_compressed;
}


void ProducerCore::WriteToSharedMemory() {
    auto start_time = std::chrono::steady_clock::now();
    size_t total_written = 0;

    while (true) {
        std::unique_lock<std::mutex> lock(packet_queue_mtx_);
        is_done_compressing_block_.wait(lock, [this]() {
            return !packet_queue_.empty() || is_done_compressing_;
        });

        if (packet_queue_.empty() && is_done_compressing_) break;

        DataPacket packet = packet_queue_.front();
        packet_queue_.pop();
        lock.unlock();

        writer_.Write(packet);
        total_written += sizeof(DataPacket);
    }
    writer_.WaitForEmpty();

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    write_time_ = duration.count();


    PrintTotalTime();
}

void ProducerCore::PrintTotalTime() {
    long long total_time = read_time_ + compress_time_ + write_time_;
    std::cout << "\n========================================" << std::endl;
    std::cout << "[PRODUCER] TOTAL TIME SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Read from file:    " << read_time_ << " ms" << std::endl;
    std::cout << "Compress data:     " << compress_time_ << " ms" << std::endl;
    std::cout << "Write to SHM:      " << write_time_ << " ms" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "TOTAL:             " << total_time << " ms" << std::endl;
    std::cout << "========================================" << std::endl;

    // Сжатие в процентах
    float compression_ratio = (float) total_compressed_bytes_ / (read_time_ * 1024);
    std::cout << "Compression ratio: " << compression_ratio << "x" << std::endl;
}
