#pragma once
#include "channel_data.h"

class ICompressor {
    public:
    ICompressor()  = default;
    virtual ~ICompressor() = default;

    virtual DataPacket Compress(DataPacket data) = 0;
};
