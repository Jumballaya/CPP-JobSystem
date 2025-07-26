#pragma once

#include <cstddef>
#include <span>

#include "FrameArena.hpp"

template <typename T, size_t N>
class ArenaArray {
 public:
  explicit ArenaArray(FrameArena* arena) : _arena(arena), _data(_arena->allocate<T>(N)) {
    assert(_data && "Arena out of memory");
  }

  ~ArenaArray() = default;
  ArenaArray(const ArenaArray&) noexcept = delete;
  ArenaArray& operator=(const ArenaArray&) noexcept = delete;

  ArenaArray(ArenaArray&& other) noexcept
      : _arena(other._arena), _data(other._data), _size(other._size) {
    other._arena = nullptr;
    other._data = nullptr;
    other._size = 0;
  }

  ArenaArray& operator=(ArenaArray&& other) noexcept {
    if (&other == this) {
      return *this;
    }
    _arena = other._arena;
    _data = other._data;
    _size = other._size;
    other._arena = nullptr;
    other._data = nullptr;
    other._size = 0;
    return *this;
  }

  T& operator[](size_t i) {
    assert(i < _size);
    return _data[i];
  }

  const T& operator[](size_t i) const {
    assert(i < _size);
    return _data[i];
  }

  T* begin() { return _data; }
  const T* begin() const { return _data; }

  T* end() { return _data + _size; }
  const T* end() const { return _data + _size; }

  std::span span() { return {_data, _size}; }
  const T* data() const { return _data; }
  size_t size() const { return _size; }
  size_t capacity() const { return N; }
  bool empty() const { return _size == 0; }

 private:
  FrameArena* _arena;
  T* _data;
  size_t _size = 0;
};