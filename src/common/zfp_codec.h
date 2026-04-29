#pragma once

#include "channel_data.h"


class ZfpCodec {
public:
    ZfpCodec() = default;

    static DataPacket Compress(const RawData &raw_data);

    static RawData Decompress(const DataPacket &compressed_data);
};
