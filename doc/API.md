# Mempool Library API Documentation

## Overview

The `Mempool` library provides a dynamic memory pool for Arduino platforms, designed for efficient allocation and deallocation of fixed-size memory segments. It is particularly useful for resource-constrained devices like the ESP32.

## Classes and Structures

### `segment`

A structure defining a memory segment with a cell count and size.

- **Constructor**:
  ```cpp
  segment(uint16_t c, uint16_t s)
  ```
  - `c`: Number of cells in the segment.
  - `s`: Size of each cell in units of `SEGMENT_STEP` (default: 4 bytes).

- **Members**:
  - `uint16_t count`: Number of cells in the segment.
  - `uint8_t size`: Size of each cell in units of `SEGMENT_STEP`.

### `mempool`

A class for managing a pool of fixed-size memory segments.

#### Constructor

```cpp
mempool()
```

Initializes the memory pool with safe defaults.

#### Destructor

```cpp
~mempool()
```

Frees all allocated resources by calling `clean()`.

#### Methods

- **begin**:
  ```cpp
  bool begin(segment* segs, uint8_t count)
  ```
  - Initializes the memory pool with an array of segments.
  - `segs`: Array of `segment` structures.
  - `count`: Number of segments (max 64).
  - Returns `true` if successful, `false` otherwise.

- **clean**:
  ```cpp
  void clean()
  ```
  - Frees all dynamically allocated memory and resets the pool.

- **alloc**:
  ```cpp
  uint8_t* alloc(uint16_t size)
  ```
  - Allocates a memory block of the specified size.
  - `size`: Size in bytes.
  - Returns a pointer to the allocated memory or `nullptr` if allocation fails.

- **alloc (template)**:
  ```cpp
  template <typename T>
  T* alloc(uint8_t count)
  ```
  - Allocates memory for an array of `count` elements of type `T`.
  - Returns a pointer to the allocated memory or `nullptr` if allocation fails.
  - Must be released using `release<T>`.

- **release**:
  ```cpp
  void release(uint8_t* ptr)
  ```
  - Releases a previously allocated memory block.
  - `ptr`: Pointer to the memory block.
  - Invalid pointers are ignored in non-debug mode.

- **release (template)**:
  ```cpp
  template <typename T>
  void release(T* ptr)
  ```
  - Releases a memory block allocated by `alloc<T>`.
  - `ptr`: Pointer to the memory block.
  - Invalid pointers are ignored in non-debug mode.

- **print_buffer**:
  ```cpp
  void print_buffer(uint8_t f = 2)
  ```
  - Prints the memory buffer content to Serial.
  - `f`: Output format (e.g., 2 for binary, 10 for decimal, 16 for hex).

- **print_pool**:
  ```cpp
  void print_pool(uint8_t f = 2)
  ```
  - Prints the pool buffer content to Serial.
  - `f`: Output format.

- **print_segment_lookup**:
  ```cpp
  void print_segment_lookup(uint8_t f = 10)
  ```
  - Prints the segment lookup table to Serial.
  - `f`: Output format.

- **print_stats**:
  ```cpp
  void print_stats()
  ```
  - Prints allocation statistics to Serial (requires `MEMPOOL_DEBUG`).

## Constants

- `SEGMENT_STEP`: Step size for segment allocation (default: 4 bytes).
- `SEGMENT_LOG2`: Log2 of `SEGMENT_STEP` (default: 2).
- `MEMPOOL_DEBUG`: Define to enable debug statistics.

## Example

```cpp
#include <mempool.h>

void setup() {
  Serial.begin(115200);
  segment segs[] = { segment(10, 4), segment(5, 8) };
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