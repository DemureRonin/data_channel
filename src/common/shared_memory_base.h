#pragma once
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdexcept>


class SharedMemoryBase {
protected:
    explicit SharedMemoryBase(bool create) {
        int flags = O_RDWR;
        if (create) flags |= O_CREAT;

        fd_ = shm_open(SHM_NAME, flags, SHM_MODE);
        if (fd_ == -1) throw std::runtime_error("shm_open failed");

        if (create) ftruncate(fd_, SHM_SIZE);

        memory_ = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd_, 0);
        if (memory_ == MAP_FAILED) throw std::runtime_error("mmap failed");
    }

public:
    virtual ~SharedMemoryBase() {
        munmap(memory_, SHM_SIZE);
        close(fd_);
    }

    [[nodiscard]] void *getMemory() const { return memory_; }

private:
    int fd_;
    void *memory_;

    static constexpr const char *SHM_NAME = "/data_channel";
    static constexpr mode_t SHM_MODE = 0640;
};
