#pragma once

#include "channel_data.h"
#include "logger.h"
#include "semaphore.h"
#include "shared_memory_base.h"

class SharedMemoryProducer : public SharedMemoryBase {
public:
    SharedMemoryProducer()
        : SharedMemoryBase(true),
          empty_(SEM_EMPTY, 1, true),
          full_(SEM_FULL, 0, true) {
    }

    void Write(const DataPacket &packet) {
        empty_.Wait();
        memcpy(getMemory(), &packet, sizeof(DataPacket));
        full_.Post();
    }

    void WaitForEmpty() {
        empty_.Wait();
    }

    ~SharedMemoryProducer() override {
        shm_unlink(SHARED_MEM_NAME.c_str());
    }

private:
    Semaphore empty_;
    Semaphore full_;
};
