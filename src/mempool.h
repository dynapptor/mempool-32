#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>

/**
 * @brief Structure to define a memory segment with count and size.
 */
struct segment {
  /**
   * @brief Constructor for segment.
   * @param c Number of cells in the segment.
   * @param s Size of each cell in the segment (in units of SEGMENT_STEP).
   */
  segment(uint16_t c, uint16_t s) : count(c), size(s) {}
  uint16_t count;  ///< Number of cells in the segment.
  uint8_t size;    ///< Size of each cell (in units of SEGMENT_STEP).
};

/**
 * @brief Memory pool class for dynamic memory management on Arduino.
 * @details Manages a pool of fixed-size memory segments for efficient allocation and deallocation.
 *          Segments are sorted by size in strictly increasing order during initialization.
 */
class mempool {
 public:
  /**
   * @brief Default constructor for mempool.
   * @details Initializes all member variables to safe defaults.
   */
  mempool();

  /**
   * @brief Destructor for mempool, cleans up allocated resources.
   */
  ~mempool();

  /**
   * @brief Frees all dynamically allocated memory and resets the pool.
   */
  void clean();

  /**
   * @brief Initializes the memory pool with given segments.
   * @param segs Array of segments to initialize the pool.
   * @param count Number of segments (must be <= 64).
   * @return True if initialization is successful, false otherwise.
   */
  bool begin(segment* segs, uint8_t count);

  /**
   * @brief Prints the buffer content to Serial.
   * @param f Format of the output (e.g., 2 for binary, 10 for decimal, 16 for hex).
   */
  void print_buffer(uint8_t f = 2);

  /**
   * @brief Prints the pool buffer content to Serial.
   * @param f Format of the output (e.g., 2 for binary, 10 for decimal, 16 for hex).
   */
  void print_pool(uint8_t f = 2);

  /**
   * @brief Prints the segment lookup table to Serial.
   * @param f Format of the output (e.g., 2 for binary, 10 for decimal, 16 for hex).
   */
  void print_segment_lookup(uint8_t f = 10);

  /**
   * @brief Prints memory pool statistics to Serial.
   * @details Requires MEMPOOL_DEBUG to be defined to print detailed statistics.
   */
  void print_stats();

  /**
   * @brief Allocates a memory block of the specified size.
   * @param size Size of the memory block to allocate (in bytes).
   * @return Pointer to the allocated memory, or nullptr if allocation fails.
   */
  uint8_t* alloc(uint16_t size);

  /**
   * @brief Template method to allocate memory for an array of type T.
   * @tparam T Type of the elements to allocate.
   * @param count Number of elements to allocate.
   * @return Pointer to the allocated memory, or nullptr if allocation fails.
   * @note The returned pointer must be released using release<T>.
   */
  template <typename T>
  T* alloc(uint8_t count);

  /**
   * @brief Releases a previously allocated memory block.
   * @param ptr Pointer to the memory block to release.
   * @note The pointer must be a valid address returned by alloc, otherwise behavior is undefined.
   */
  void release(uint8_t* ptr);

  /**
   * @brief Template method to release a previously allocated memory block of type T.
   * @tparam T Type of the elements to release.
   * @param ptr Pointer to the memory block allocated by alloc<T>.
   * @note The pointer must be a valid address returned by alloc<T>, otherwise behavior is undefined.
   */
  template <typename T>
  void release(T* ptr);

  /**
   * @brief Return the biggest sigment size.
   */
  uint16_t max_segment_size();

 private:
  bool _initialized = false;         ///< Flag indicating if the pool is initialized.
  uint8_t* _buffer = nullptr;        ///< Buffer for memory pool.
  uint32_t _buffer_size = 0;         ///< Total size of the buffer in bytes.
  uint16_t _pool_size = 0;           ///< Size of the pool buffer in 32-bit words.
  uint32_t* _pool_buffer = nullptr;  ///< Buffer for pool allocation masks.
  uint32_t** _pool_ptr = nullptr;    ///< Pointers to pool mask starts for each segment.
  uint8_t** _segment_ptr = nullptr;  ///< Pointers to segment starts in _buffer.

  uint16_t _max_segment_size = 0;      ///< Maximum segment size in bytes.
  uint16_t* _segment_sizes = nullptr;  ///< Array of segment sizes in bytes.
  uint16_t* _cell_count = nullptr;     ///< Array of cell counts per segment.
  uint32_t* _magic_number = nullptr;   ///< Magic numbers for fast division when segment size is not a power of 2.
  uint8_t* _segment_shift = nullptr;   ///< Shift values for fast division when segment size is a power of 2.

  uint8_t _segment_count = 0;          ///< Number of segments.
  int16_t* _segment_lookup = nullptr;  ///< Lookup table for segment selection.
  uint16_t _segment_lookup_count = 0;  ///< Number of entries in the segment lookup table.

  SemaphoreHandle_t _mutex = nullptr;
  /**
   * @brief Finds the next segment with size greater than current.
   * @param arr Array of segments.
   * @param count Number of segments.
   * @param current Current size to compare.
   * @return Index of the next segment with smallest size greater than current.
   */
  uint8_t _get_next_segment(segment* arr, uint8_t count, uint16_t current);

  /**
   * @brief Looks up the segment suitable for a given size.
   * @param size Size to find a segment for (in bytes).
   * @return Index of the segment, or -1 if none found.
   */
  int16_t _lookup_segment(uint16_t size);

  /**
   * @brief Prepares a bit mask for a given cell count.
   * @param c Number of cells.
   * @return Bit mask with the first c bits set to 0 (indicating free cells) and the rest to 1.
   */
  uint32_t _prepare_mask(uint8_t c);

#ifdef MEMPOOL_DEBUG
  uint16_t* _max_cells_used = nullptr;      ///< Maximum cells used per segment (debug only).
  uint32_t _total_allocs = 0;               ///< Total number of allocations (debug only).
  uint32_t _failed_allocs = 0;              ///< Number of failed allocations (debug only).
  uint32_t* _allocs_per_segment = nullptr;  ///< Allocations per segment (debug only).
#endif
};

#include "mempool.tpp"

extern mempool mem;