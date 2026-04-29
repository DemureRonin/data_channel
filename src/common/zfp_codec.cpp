#include <zfp.h>
#include <cstring>
#include <memory>

#include "zfp_codec.h"


DataPacket ZfpCodec::Compress(const RawData &raw_data) {
    const float *data = reinterpret_cast<const float *>(raw_data.buffer.data());
    size_t num_floats = raw_data.size / sizeof(float);

    zfp_field *field = zfp_field_1d(const_cast<float *>(data), zfp_type_float, num_floats);
    zfp_stream *zfp = zfp_stream_open(nullptr);
    zfp_stream_set_rate(zfp, COMPRESSION_FACTOR, zfp_type_float, 1, 0);

    size_t bufsize = zfp_stream_maximum_size(zfp, field);

    DataPacket result;
    std::vector<char> temp_buffer(bufsize);

    bitstream *bs = stream_open(temp_buffer.data(), temp_buffer.size());
    zfp_stream_set_bit_stream(zfp, bs);
    zfp_stream_rewind(zfp);

    size_t compressed_size = zfp_compress(zfp, field);

    if (compressed_size > MAX_PACKET_BUFFER_SIZE) {
        throw std::runtime_error("Compressed data exceeds SHM packet size");
    }

    memcpy(result.buffer.data(), temp_buffer.data(), compressed_size);
    result.size = compressed_size;
    result.is_last = raw_data.is_last;
    result.original_count = num_floats;

    stream_close(bs);
    zfp_stream_close(zfp);
    zfp_field_free(field);

    return result;
}

RawData ZfpCodec::Decompress(const DataPacket &compressed_data) {
    if (compressed_data.size == 0) {
        throw std::runtime_error("Empty compressed data");
    }

    size_t num_floats = compressed_data.original_count;
    size_t decompressed_bytes = num_floats * sizeof(float);


    zfp_field *field = zfp_field_1d(nullptr, zfp_type_float, num_floats);
    zfp_stream *zfp = zfp_stream_open(nullptr);

    bitstream *bs = stream_open(const_cast<char *>(compressed_data.buffer.data()),
                                compressed_data.size);
    zfp_stream_set_bit_stream(zfp, bs);
    zfp_stream_rewind(zfp);

    RawData result;
    result.buffer.resize(decompressed_bytes);
    result.size = decompressed_bytes;
    result.original_count = num_floats;
    result.is_last = compressed_data.is_last;
    field->data = result.buffer.data();

    size_t decompressed_size = zfp_decompress(zfp, field);

    if (decompressed_size == 0) {
        throw std::runtime_error("Decompression failed");
    }

    stream_close(bs);
    zfp_stream_close(zfp);
    zfp_field_free(field);

    return result;
}
