# Data Channel

High-performance compressed data channel for interprocess communication using shared memory.

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_C_COMPILER=clang \
         -DCMAKE_CXX_COMPILER=clang++ \
         -DCMAKE_CXX_STANDARD=17 \
         -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"
make -j4
```

## Run options

./producer <input_file> [--no-compress]

./consumer <output_file> [--no-compress]

#### Recommended to run simultaneously, as consumer waits for semaphores and shared memory

./producer <input_file> [--no-compress] & ./consumer <output_file> [--no-compress]


### Compression Algorithm Choice

ZFP library was chosen because it guarantees deterministic output size. With fixed-rate mode (7 bits per value), 280 floats (1120 bytes) compress to ~245 bytes, fitting perfectly into the 248-byte SHM
payload. Loss is allowed and controlled.

### Synchronization Methods

Three POSIX semaphores are used:

- `sem_empty` (initial 1) – controls write access
- `sem_full` (initial 0) – signals data available
- `sem_done` – notifies Producer when Consumer finishes

Shared memory is a fixed 256-byte segment. Producer writes via `memcpy`, Consumer reads.

Producer waits for empty - writes - signals full. Consumer waits for full - reads - signals empty.

### Compromises

Fixed-rate compression wastes ~8 bytes in SHM (payload 248 bytes, metadata 8 bytes).  Lossy compression gives 4x ratio with ~7% error.

### Optimizations

Three-thread pipeline (read file - compress - write to SHM) overlaps I/O, compression, and SHM operations. Bitrate = 7 was manually tuned to keep compressed size under 248 bytes.

## Cleanup
Application gracefully handles argument errors and interrupts.

If Producer fails, consumer waits for 5 seconds before timing out.

Nonetheless, it is recommended to check **if all semaphores are cleared and processes are dead** in case of bad data or error.

### TODO

- Implement SDL graphical interface
- Use thread pool to avoid code duplication and split decompression into more threads
