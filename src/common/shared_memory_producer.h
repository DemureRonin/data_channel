#pragma once
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdexcept>
#include "channel_data.h"
#include "semaphore.h"

class SharedMemoryProducer {
public:
    SharedMemoryProducer() : empty_(SEM_EMPTY, 1, true), full_(SEM_FULL, 0, true) {
        shm_unlink(SHARED_MEM_NAME.c_str());


        fd_ = shm_open(SHARED_MEM_NAME.c_str(), O_CREAT | O_RDWR, 0640);
        if (fd_ == -1) throw std::runtime_error("shm_open failed");

        ftruncate(fd_, SHM_SIZE);

        memory_ = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (memory_ == MAP_FAILED) throw std::runtime_error("mmap failed");
    }

    ~SharedMemoryProducer() {
        munmap(memory_, SHM_SIZE);
        close(fd_);

        shm_unlink(SHARED_MEM_NAME.c_str());

        sem_unlink(SEM_EMPTY.c_str());
        sem_unlink(SEM_FULL.c_str());
    }

    void Write(const DataPacket &packet) {
        empty_.Wait();
        memcpy(memory_, &packet, sizeof(DataPacket));
        full_.Post();
    }

    void WaitForEmpty() {
        empty_.Wait();
    }

private:
    int fd_;
    void *memory_;
    Semaphore empty_;
    Semaphore full_;
};
