#include "mempool.h"

#include <Arduino.h>

mempool::mempool() {}

mempool::~mempool() {
  clean();
}

void mempool::clean() {
  if (_segment_sizes) delete[] _segment_sizes;
  if (_cell_count) delete[] _cell_count;
  if (_magic_number) delete[] _magic_number;
  if (_segment_shift) delete[] _segment_shift;
  if (_buffer) delete[] _buffer;
  if (_pool_buffer) delete[] _pool_buffer;
  if (_segment_lookup) delete[] _segment_lookup;
  if (_segment_ptr) delete[] _segment_ptr;
  if (_pool_ptr) delete[] _pool_ptr;
#ifdef MEMPOOL_DEBUG
  if (_max_cells_used) delete[] _max_cells_used;
  if (_allocs_per_segment) delete[] _allocs_per_segment;
#endif
}

bool mempool::begin(segment* segs, uint8_t count) {
  if (_initialized) return false;
  if (count > 64) return false;

  // Allocate arrays with nullptr checks
  _segment_sizes = new uint16_t[count];
  if (!_segment_sizes) {
    clean();
    return false;
  }
  _cell_count = new uint16_t[count];
  if (!_cell_count) {
    clean();
    return false;
  }
  _magic_number = new uint32_t[count];
  if (!_magic_number) {
    clean();
    return false;
  }
  _segment_shift = new uint8_t[count]{};
  if (!_segment_shift) {
    clean();
    return false;
  }
  _segment_ptr = new uint8_t*[count];
  if (!_segment_ptr) {
    clean();
    return false;
  }
  _pool_ptr = new uint32_t* [count] {};
  if (!_pool_ptr) {
    clean();
    return false;
  }
#ifdef MEMPOOL_DEBUG
  _max_cells_used = new uint16_t[count]{};
  if (!_max_cells_used) {
    clean();
    return false;
  }
  _allocs_per_segment = new uint32_t[count]{};
  if (!_allocs_per_segment) {
    clean();
    return false;
  }
#endif
  _initialized = true;
  _segment_count = count;
  _buffer_size = 0;

  uint8_t ix;
  uint16_t currentSize = 0;
  for (uint8_t i = 0; i < count; ++i) {
    if (segs[i].size == 0) {
      clean();
      return false;
    }
    ix = _get_next_segment(segs, count, currentSize);

    _segment_sizes[i] = segs[ix].size * SEGMENT_STEP;
    if (_segment_sizes[i] > 64) {
      clean();
      return false;
    }
    _cell_count[i] = segs[ix].count;

    currentSize = segs[ix].size;
    _buffer_size += _segment_sizes[i] * _cell_count[i];  // Data buffer size
    _pool_size += (_cell_count[i] + 31) / 32;            // Pool mask size
    _pool_size++;                                        // Pool header mask
  }

  _max_segment_size = _segment_sizes[count - 1];

  _buffer = new uint8_t[_buffer_size]{};
  if (!_buffer) {
    clean();
    return false;
  }
  _pool_buffer = new uint32_t[_pool_size]{};
  if (!_pool_buffer) {
    clean();
    return false;
  }
  _segment_lookup_count = (_max_segment_size / SEGMENT_STEP);
  _segment_lookup = new int16_t[_segment_lookup_count];
  if (!_segment_lookup) {
    clean();
    return false;
  }

  // Initialize segment lookup table (O(n*m) but initialization speed is not critical)
  for (uint16_t i = 1; i <= _segment_lookup_count; i++) {
    _segment_lookup[i - 1] = _lookup_segment(i * SEGMENT_STEP);
  }

  // Initialize segment and pool pointers
  _segment_ptr[0] = _buffer;
  _pool_ptr[0] = _pool_buffer;
  for (uint8_t i = 0; i < count - 1; ++i) {
    _segment_ptr[i + 1] = _segment_ptr[i] + _segment_sizes[i] * _cell_count[i];
    _pool_ptr[i + 1] = _pool_ptr[i] + (_cell_count[i] + 31) / 32 + 1;
  }

  // Initialize magic numbers and shifts for fast division
  for (uint8_t i = 0; i < count; ++i) {
    if (_segment_sizes[i] & (_segment_sizes[i] - 1)) {
      // Non-power-of-2 sizes use magic number for fast division
      _magic_number[i] = (65536 + (_segment_sizes[i] >> 2) - 1) / (_segment_sizes[i] >> 2);
      _segment_shift[i] = 16;
    } else {
      // Power-of-2 sizes use bit shift for fast division
      _magic_number[i] = 1;
      _segment_shift[i] = __builtin_ctz(_segment_sizes[i]);
    }
  }

  // Initialize pool masks (0 bits indicate free cells)
  for (uint8_t i = 0; i < count; i++) {
    _pool_ptr[i][0] = _prepare_mask((_cell_count[i] + 31) / 32);
    _pool_ptr[i][(_cell_count[i] + 31) / 32] = _prepare_mask(_cell_count[i] % 32);
  }
  return true;
}

uint8_t mempool::_get_next_segment(segment* arr, uint8_t count, uint16_t current) {
  uint16_t find = -1;
  uint8_t ret = 0;
  for (uint8_t i = 0; i < count; i++) {
    if (arr[i].size > current && arr[i].size < find) {
      ret = i;
      find = arr[i].size;
    }
  }
  return ret;
}

int16_t mempool::_lookup_segment(uint16_t size) {
  for (uint8_t i = 0; i < _segment_count; i++) {
    if (_segment_sizes[i] >= size) return i;
  }
  return -1;
}

uint32_t mempool::_prepare_mask(uint8_t c) {
  if (c == 0) return 0;
  uint32_t ret = 0xFFFFFFFF;
  ret = ret << c;
  return ret;
}

void mempool::print_buffer(uint8_t f) {
  if (!Serial) return;
  for (uint32_t i = 0; i < _buffer_size; i++) {
    Serial.print(_buffer[i], f);
    Serial.print(' ');
  }
  Serial.println();
}

void mempool::print_pool(uint8_t f) {
  if (!Serial) return;
  for (uint32_t i = 0; i < _pool_size; i++) {
    Serial.print(_pool_buffer[i], f);
    Serial.print(' ');
  }
  Serial.println();
}

void mempool::print_segment_lookup(uint8_t f) {
  if (!Serial) return;
  for (uint16_t i = 0; i < _segment_lookup_count; i++) {
    Serial.print(_segment_lookup[i], f);
    Serial.print(' ');
  }
  Serial.println();
}

void mempool::print_stats() {
  if (!Serial) return;
#ifdef MEMPOOL_DEBUG
  Serial.print("Total allocs: ");
  Serial.println(_total_allocs);
  Serial.print("Failed allocs: ");
  Serial.println(_failed_allocs);
  for (uint8_t i = 0; i < _segment_count; i++) {
    Serial.print("Segment ");
    Serial.print(i);
    Serial.print(": max cells used = ");
    Serial.print(_max_cells_used[i]);
    Serial.print(", allocs = ");
    Serial.println(_allocs_per_segment[i]);
  }
#else
  Serial.println("Debug stats not available. Enable MEMPOOL_DEBUG to see statistics.");
#endif
}

uint8_t* mempool::alloc(uint16_t size) {
  if (size > _max_segment_size) {
#ifdef MEMPOOL_DEBUG
    _failed_allocs++;
#endif
    return nullptr;
  }
  uint8_t sg = _segment_lookup[((size + SEGMENT_STEP - 1) >> SEGMENT_LOG2) - 1];
  if (sg >= _segment_count) {
#ifdef MEMPOOL_DEBUG
    _failed_allocs++;
#endif
    return nullptr;
  }

  uint8_t* p = _segment_ptr[sg];

  if (*_pool_ptr[sg] == 0xFFFFFFFF) {
    if (sg < _segment_count - 1) {
      return alloc(_segment_sizes[sg + 1]);
    } else {
#ifdef MEMPOOL_DEBUG
      _failed_allocs++;
#endif
      return nullptr;
    }
  }
  uint8_t pool_index = __builtin_ctz(~_pool_ptr[sg][0]);
  if (pool_index >= (_cell_count[sg] + 31) / 32) {
#ifdef MEMPOOL_DEBUG
    _failed_allocs++;
#endif
    return nullptr;
  }
  uint32_t* cell_mask = &_pool_ptr[sg][pool_index + 1];

  uint8_t cell_index = __builtin_ctz(~*cell_mask);
  bitSet(*cell_mask, cell_index);
  if (*cell_mask == 0xFFFFFFFF) {
    bitSet(*_pool_ptr[sg], pool_index);
  }
#ifdef MEMPOOL_DEBUG
  _total_allocs++;
  _allocs_per_segment[sg]++;
  uint16_t used_cells = pool_index * 32 + cell_index;
  if (used_cells > _max_cells_used[sg]) _max_cells_used[sg] = used_cells;
#endif
  return _segment_ptr[sg] + (pool_index * 32 + cell_index) * _segment_sizes[sg];
}

void mempool::release(uint8_t* ptr) {
  if (!_initialized || !ptr || ptr < _buffer || ptr >= _buffer + _buffer_size) {
    return;
  }

  // Binary search for segment (O(log n), efficient for large segment counts)
  int8_t sg = -1;
  uint8_t l = 0, r = _segment_count - 1;
  while (l <= r) {
    uint8_t m = (l + r) >> 1;
    if (ptr < _segment_ptr[m]) {
      r = m - 1;
    } else {
      sg = m;
      l = m + 1;
    }
  }
  if (sg == -1) {
    return;
  }

  uint8_t* base = _segment_ptr[sg];
  uint16_t offset = ptr - base;
  uint8_t cellIndex;
  if (_segment_sizes[sg] & (_segment_sizes[sg] - 1)) {
    // Non-power-of-2 sizes use magic number for fast division
    cellIndex = ((offset >> 2) * _magic_number[sg]) >> 16;
  } else {
    // Power-of-2 sizes use bit shift for fast division
    cellIndex = offset >> _segment_shift[sg];
  }
  uint8_t poolIndex = cellIndex >> 5;
  uint8_t bitIndex = cellIndex & 31;

  uint32_t* pp = _pool_ptr[sg];
  bitClear(*pp, poolIndex);
  pp += poolIndex + 1;
  bitClear(*pp, bitIndex);
}
