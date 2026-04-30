#pragma once

#include "channel_data.h"

/**
 * @brief ZFP codec: compress/decompress float arrays (fixed-rate 7 bits/value)
 */
class ZfpCodec {
public:
    ZfpCodec() = default;

    /**
     * @brief Compress float array → DataPacket
     * @param raw_data Input with float data
     * @return DataPacket Compressed bitstream + metadata
     * @throws runtime_error on failure
     */
    static DataPacket Compress(const RawData &raw_data);

    /**
     * @brief Decompress DataPacket → float array
     * @param compressed_data Input packet with compressed bitstream
     * @return RawData Restored float data
     * @throws runtime_error on failure
     */
    static RawData Decompress(const DataPacket &compressed_data);
};