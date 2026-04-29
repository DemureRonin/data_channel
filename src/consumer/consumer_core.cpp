#include "consumer_core.h"
#include <fstream>

void ConsumerCore::ReadFromSharedMemory() {
    while (true) {
        DataPacket packet = reader_.Read();

        {
            std::lock_guard lock(packet_queue_mtx_);
            packet_queue_.push(packet);
        }
        packet_cv_.notify_one();

        if (packet.is_last) break;
    }
    is_done_reading_ = true;
    packet_cv_.notify_all();
}

void ConsumerCore::HandleDecompress() {
    while (true) {
        std::unique_lock<std::mutex> lock(packet_queue_mtx_);
        packet_cv_.wait(lock, [this]() {
            return !packet_queue_.empty() || is_done_reading_;
        });

        if (packet_queue_.empty() && is_done_reading_) break;

        DataPacket packet = packet_queue_.front();
        packet_queue_.pop();
        lock.unlock();

        RawData decompressed;
        if (compress_) {
            decompressed = ZfpCodec::Decompress(packet);
        } else {
            decompressed.buffer.resize(packet.size);
            memcpy(decompressed.buffer.data(), packet.buffer.data(), packet.size);
            decompressed.size = packet.size;
            decompressed.original_count = packet.original_count;
            decompressed.is_last = packet.is_last;
        }

        {
            std::lock_guard raw_lock(raw_data_queue_mtx_);
            raw_data_queue_.push(decompressed);
        }
        raw_data_cv_.notify_one();
    }
    is_done_decompressing_ = true;
    raw_data_cv_.notify_all();
}

void ConsumerCore::WriteToFile() {
    std::ofstream file(output_file_, std::ios::binary);

    while (true) {
        std::unique_lock<std::mutex> lock(raw_data_queue_mtx_);
        raw_data_cv_.wait(lock, [this]() {
            return !raw_data_queue_.empty() || is_done_decompressing_;
        });

        if (raw_data_queue_.empty() && is_done_decompressing_) break;

        RawData data = raw_data_queue_.front();
        raw_data_queue_.pop();
        lock.unlock();

        file.write(data.buffer.data(), data.size);

        if (data.is_last) break;
    }

    file.close();
}
