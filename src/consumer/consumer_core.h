#pragma once
#include <thread>
#include <fstream>
#include "../common/shared_memory_consumer.h"
#include "../common/zfp_codec.h"
#include "../common/logger.h"

class ConsumerCore {
public:
    ConsumerCore(const std::string &output_file) : output_file_(output_file) {
        read_thread_ = std::thread(&ConsumerCore::Run, this);
    }

    ~ConsumerCore() {
        if (read_thread_.joinable()) read_thread_.join();
    }

    void Run() {
        std::ofstream file(output_file_, std::ios::binary);
        while (true) {
            DataPacket packet = reader_.Read();
            RawData decompressed = ZfpCodec::Decompress(packet);
            file.write(decompressed.buffer.data(), decompressed.size);

            if (packet.is_last) {
                break;
            }
        }
        file.close();
    }

private:
    std::string output_file_;
    SharedMemoryConsumer reader_;

    std::thread read_thread_;
};
