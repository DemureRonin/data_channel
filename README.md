# Data Channel

High-performance compressed data channel for interprocess communication using shared memory.

## Build

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_C_COMPILER=clang \
         -DCMAKE_CXX_COMPILER=clang++
make -j4
```
## Run options

./producer <input_file> [--no-compress]

./consumer <output_file> [--no-compress]

#### Recommended to run simultaneously, as consumer waits for semaphores and shared memory
./producer <input_file> [--no-compress] & ./consumer <output_file> [--no-compress]

## Documentation

### Compression Algorithm Choice

The ZFP library was chosen for compression. It is reliable, open-source and specifically designed for floating-point data. The main reason is deterministic output size. ZFP fixed-rate mode guarantees that for a given input block of 1120 bytes (280 floats) and bitrate of 7 bits per value, the compressed output size is always constant at approximately 245 bytes. This fits perfectly into the 248 byte SHM payload. ZFP also allows controlled precision loss, which is acceptable according to the task requirements.

### Synchronization Methods

POSIX named semaphores and shared memory were used. Three semaphores are defined. sem_empty controls write access to SHM and is initialized to 1. sem_full signals available data for reading and is initialized to 0. sem_done notifies the Producer when the Consumer has finished writing to the output file.

Shared memory is a fixed 256 byte segment. The Producer creates the segment and the Consumer opens the existing one. Data packets are copied directly using memcpy.

The synchronization flow works as follows. The Producer waits for the empty semaphore, writes the packet to SHM, then signals the full semaphore. The Consumer waits for the full semaphore, reads the packet from SHM, then signals the empty semaphore. At the end of transmission, the done semaphore ensures the Producer knows the Consumer has finished.

### Compromises

Fixed rate compression leaves a few bytes unused in the SHM segment. The payload is 248 bytes and metadata uses approximately 8 bytes. This small waste is an acceptable trade off for predictable behavior and simplified channel management.

Lossy compression with ZFP achieves about 4x compression ratio, reducing the 50 MB file to approximately 11 MB. The controlled error is around 0.1 to 1 percent, which significantly reduces transfer time while loss is explicitly allowed by the task.

### Optimizations

A three thread pipeline was implemented with separate threads for reading the file, compressing data, and writing to SHM. This overlaps I/O, compression, and SHM operations for better throughput at the cost of additional mutex and condition variable complexity.

The bitrate parameter was manually tuned to 7 bits per value to guarantee that compressed size stays below 248 bytes for all input data patterns while maintaining acceptable numerical accuracy.

The Consumer removes semaphores as the last user and the Producer removes the SHM segment. This prevents resource leaks even if one process terminates unexpectedly.

### TODO
- Implement graphical SDL interface
- Implement thread pool to avoid code duplication and split up decompression process into more threads

