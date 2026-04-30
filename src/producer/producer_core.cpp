#include "producer_core.h"

#include "../common/zfp_codec.h"
#include <fstream>
#include <chrono>

void ProducerCore::HandleReadFromFile() {
    auto start_time = std::chrono::steady_clock::now();
    std::ifstream file(input_file_, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("failed to open file: " + input_file_);

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
        raw_data_cv_.notify_one();

        if (raw_data.is_last) break;
    }

    total_bytes_read_ = bytes_read_total;
    is_done_reading_ = true;
    raw_data_cv_.notify_all();

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
            std::lock_guard lock(compressed_queue_mtx_);
            compressed_queue_.push(packet);
        }
        compressed_cv_.notify_one();

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
        std::unique_lock<std::mutex> lock(raw_data_queue_mtx_);
        raw_data_cv_.wait(lock, [this]() {
            return !raw_data_queue_.empty() || is_done_reading_;
        });

        if (raw_data_queue_.empty() && is_done_reading_) break;

        auto raw_data = std::move(raw_data_queue_.front());
        raw_data_queue_.pop();
        lock.unlock();

        if (compress_) {
            auto compressed = ZfpCodec::Compress(raw_data);
            total_compressed += compressed.size;
            {
                std::lock_guard compressed_lock(compressed_queue_mtx_);
                compressed_queue_.push(std::move(compressed));
            }
            compressed_cv_.notify_one();
        } else {
            total_compressed += SplitIntoPackets(raw_data);
        }
    }

    is_done_compressing_ = true;
    compressed_cv_.notify_all();

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    compress_time_ = duration.count();
    total_compressed_bytes_ = total_compressed;
}

size_t ProducerCore::CompareFiles() {
    std::ifstream original(input_file_, std::ios::binary);
    if (!original.is_open()) throw std::runtime_error("failed to open file: " + input_file_);
    std::ifstream restored(output_file_, std::ios::binary);
    if (!restored.is_open()) throw std::runtime_error("failed to open file: " + output_file_);

    original.seekg(0, std::ios::end);
    restored.seekg(0, std::ios::end);
    size_t original_size = original.tellg();
    size_t restored_size = restored.tellg();

    original.seekg(0, std::ios::beg);
    restored.seekg(0, std::ios::beg);

    size_t num_floats = std::min(original_size, restored_size) / sizeof(float);

    double total_absolute_error = 0.0;
    double total_original_value = 0.0;
    double max_absolute_error = 0.0;

    for (size_t i = 0; i < num_floats; ++i) {
        float orig_val, rest_val;
        original.read(reinterpret_cast<char *>(&orig_val), sizeof(float));
        restored.read(reinterpret_cast<char *>(&rest_val), sizeof(float));

        double abs_error = std::abs(orig_val - rest_val);
        total_absolute_error += abs_error;
        total_original_value += std::abs(orig_val);
        max_absolute_error = std::max(max_absolute_error, abs_error);
    }


    double mae = total_absolute_error / num_floats;


    double avg_original = total_original_value / num_floats;
    double loss_percent = (mae / avg_original) * 100.0;


    return static_cast<size_t>(loss_percent);
}

void ProducerCore::HandleWrite() {
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        std::unique_lock<std::mutex> lock(compressed_queue_mtx_);
        compressed_cv_.wait(lock, [this]() {
            return !compressed_queue_.empty() || is_done_compressing_;
        });

        if (compressed_queue_.empty() && is_done_compressing_) break;

        DataPacket packet = std::move(compressed_queue_.front());
        compressed_queue_.pop();
        lock.unlock();

        writer_.Write(packet);
    }
    writer_.WaitForEmpty();

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    write_time_ = duration.count();
    writer_.WaitForConsumer();
    loss_ = CompareFiles();
}

TransferReport ProducerCore::Run() {
    read_thread_ = std::thread(&ProducerCore::HandleReadFromFile, this);
    compress_thread_ = std::thread(&ProducerCore::HandleCompress, this);
    write_thread_ = std::thread(&ProducerCore::HandleWrite, this);

    if (read_thread_.joinable()) read_thread_.join();
    if (compress_thread_.joinable()) compress_thread_.join();
    if (write_thread_.joinable()) write_thread_.join();

    return FormReport();
}

TransferReport ProducerCore::FormReport() const {
    TransferReport report{};
    report.read_from_file_time = read_time_;
    report.compress_time = compress_time_;
    report.write_to_shm_time = write_time_;
    report.total_time = read_time_ + compress_time_ + write_time_;
    report.bytes_read_from_file = total_bytes_read_;
    report.bytes_sent_to_shm = total_compressed_bytes_;

    if (total_compressed_bytes_ > 0) {
        report.compression_ratio = (double) total_bytes_read_ / total_compressed_bytes_;
        report.space_saved = (1.0 - (double) total_compressed_bytes_ / total_bytes_read_) * 100.0;
    } else {
        report.compression_ratio = 0;
        report.space_saved = 0;
    }

    report.loss = loss_;
    return report;
}
