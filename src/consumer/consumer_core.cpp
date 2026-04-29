#include "consumer_core.h"
#include <fstream>
#include <cstring>

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
}

void ConsumerCore::HandleDecompress() {
    while (true) {
        std::unique_lock<std::mutex> lock(packet_queue_mtx_);

        packet_cv_.wait(lock, [this]() {
            return !packet_queue_.empty();
        });

        DataPacket packet = std::move(packet_queue_.front());
        packet_queue_.pop();
        lock.unlock();

        RawData decompressed;

        if (compress_) {
            decompressed = ZfpCodec::Decompress(packet);
        } else {
            decompressed.buffer.resize(packet.size);
            std::memcpy(decompressed.buffer.data(), packet.buffer.data(), packet.size);
            decompressed.size = packet.size;
            decompressed.original_count = packet.original_count;
            decompressed.is_last = packet.is_last;
        }

        {
            std::lock_guard raw_lock(raw_data_queue_mtx_);
            raw_data_queue_.push(std::move(decompressed));
        }
        raw_data_cv_.notify_one();

        if (packet.is_last) break;
    }
}

void ConsumerCore::WriteToFile() {
    std::ofstream file(output_file_, std::ios::binary);

    if (!file.is_open()) {
        reader_.SignalDone();
        return;
    }

    while (true) {
        std::unique_lock<std::mutex> lock(raw_data_queue_mtx_);

        raw_data_cv_.wait(lock, [this]() {
            return !raw_data_queue_.empty();
        });

        RawData data = std::move(raw_data_queue_.front());
        raw_data_queue_.pop();
        lock.unlock();

        file.write(data.buffer.data(), data.size);

        if (file.fail()) {
            break;
        }

        if (data.is_last) break;
    }

    file.close();
    reader_.SignalDone();
}