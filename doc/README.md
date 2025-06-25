# Mempool Arduino Library

## Overview

The `Mempool` library provides a dynamic memory pool for efficient memory management on Arduino platforms, particularly suited for resource-constrained devices like the ESP32. It supports allocation and deallocation of fixed-size memory segments, with segments sorted by size in strictly increasing order.

## Features

- Efficient allocation and deallocation of memory blocks.
- Multiple segment sizes, each with a specified number of cells.
- Optional debug mode (`MEMPOOL_DEBUG`) for tracking allocation statistics.
- Robust error handling with nullptr checks and cleanup.

## Installation

1. Download the library from the GitHub repository.
2. Extract the `Mempool` folder to your Arduino `libraries` directory (e.g., `~/Arduino/libraries/`).
3. Include the library in your sketch with `#include <mempool.h>`.

## Usage Example

```cpp
#include <mempool.h>

void setup() {
  Serial.begin(115200);
  segment segs[] = { segment(10, 1), segment(5, 2) }; // 10x4 bytes, 5x8 bytes
  mempool pool;
  if (!pool.begin(segs, 2)) {
    Serial.println("Initialization failed!");
    return;
  }
  int* ptr = pool.alloc<int>(1);
  if (ptr) {
    *ptr = 42;
    pool.release(ptr);
  }
  pool.print_stats();
}

void loop() {}
```

## File List

- `mempool.h`: Header file defining the `mempool` class and `segment` structure.
- `mempool.cpp`: Implementation of the `mempool` class.
- `mempool.tpp`: Template definitions for `alloc` and `release` methods.
- `keywords.txt`: Keyword definitions for Arduino IDE syntax highlighting.
- `library.properties`: Metadata for the Arduino library.
- `API.md`: Detailed API documentation.
- `README.adoc`: AsciiDoc overview of the library.
- `README.md`: This file, providing setup and usage instructions.

## Notes

- Requires `Serial.begin()` for debug output functions (`print_buffer`, `print_pool`, `print_segment_lookup`, `print_stats`).
- Define `MEMPOOL_DEBUG` to enable allocation statistics.
- Maximum segment count is 64.
- Segment sizes must be multiples of `SEGMENT_STEP` (default: 4 bytes) and <= 64 bytes.

## License

MIT License. See `LICENSE` for details.