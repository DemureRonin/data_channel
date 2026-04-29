#pragma once
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <thread>
#include <chrono>
#include <stdexcept>
#include "channel_data.h"
#include "semaphore.h"

class SharedMemoryConsumer {
public:
    SharedMemoryConsumer() : empty_(SEM_EMPTY, 0, false),
                             full_(SEM_FULL, 0, false),
                             done_(SEM_DONE, 0, false) {
        bool shm_ready = false;

        constexpr int NUM_RETRIES = 50;
        constexpr int RETRIES_INTERVAL = 100;

        for (int i = 0; i < NUM_RETRIES; ++i) {
            fd_ = shm_open(SHARED_MEM_NAME.c_str(), O_RDWR, 0);
            if (fd_ != -1) {
                shm_ready = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRIES_INTERVAL));
        }

        if (!shm_ready) {
            throw std::runtime_error("Timeout waiting for SHM from Producer");
        }

        memory_ = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (memory_ == MAP_FAILED) throw std::runtime_error("mmap failed");
    }

    ~SharedMemoryConsumer() {
        munmap(memory_, SHM_SIZE);
        close(fd_);
    }

    DataPacket Read() {
        full_.Wait();
        DataPacket packet;
        memcpy(&packet, memory_, sizeof(DataPacket));
        empty_.Post();
        return packet;
    }

    void SignalDone() {
        done_.Post();
    }

private:
    int fd_;
    void *memory_;
    Semaphore empty_;
    Semaphore full_;
    Semaphore done_;
};
