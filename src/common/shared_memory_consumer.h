#pragma once
#include <iostream>

#include "channel_data.h"
#include "semaphore.h"
#include "shared_memory_base.h"


class SharedMemoryConsumer : public SharedMemoryBase {
public:
    SharedMemoryConsumer()
        : SharedMemoryBase(false),
          empty_(SEM_EMPTY, 0, false),
          full_(SEM_FULL, 0, false) {
    }

    [[nodiscard]] DataPacket Read() {
        full_.Wait();
        DataPacket packet{};


        memcpy(&packet, getMemory(), sizeof(DataPacket));

        empty_.Post();
        return packet;
    }

private:
    Semaphore empty_;
    Semaphore full_;
};
