#include <cstdlib>
#include <cstring>

#include "nanolog.hpp"
#include "memoryimpl.h"

// --

static std::shared_ptr<MemoryInterface> s_mi;

void MemoryInterface::set(std::shared_ptr<MemoryInterface> mi) { s_mi = mi; }

std::shared_ptr<MemoryInterface> MemoryInterface::get() { return s_mi; }

// --

uint8_t *MemoryManager::alloc(size_t len) const noexcept {
  uint8_t *r = (uint8_t *)std::malloc(len);
  if (!r) {
    NLOG_CRIT << "memory lack.";
    std::exit(-1);
  }
  return r;
}

void MemoryManager::free(uint8_t *p) const noexcept { std::free(p); }

uint8_t *MemoryManager::copy(uint8_t *dest, const uint8_t *src,
                             size_t count) const noexcept {
  return (uint8_t*)std::memcpy(dest, src, count);
}

uint8_t *MemoryManager::set(uint8_t *dest, int ch, size_t count) const
    noexcept {
  return (uint8_t*)std::memset(dest, ch, count);
}
