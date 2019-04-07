#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

struct MemoryInterface {
  static std::shared_ptr<MemoryInterface> get();
  static void set(std::shared_ptr<MemoryInterface> mi);

  virtual uint8_t *alloc(size_t len) const noexcept = 0;
  virtual void free(uint8_t *p) const noexcept = 0;
  virtual uint8_t *copy(uint8_t *dest, const uint8_t *src, size_t count) const
      noexcept = 0;
  virtual uint8_t *set(uint8_t *dest, int ch, size_t count) const noexcept = 0;

  virtual ~MemoryInterface() = default;
};
