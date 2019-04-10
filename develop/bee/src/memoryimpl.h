#pragma once

#include "memoryinterface.h"

class MemoryManager : public MemoryInterface {
public:
  MemoryManager();
  ~MemoryManager() override;

  uint8_t *alloc(size_t len)const noexcept override;
  void free(uint8_t *p) const noexcept override;
  uint8_t *copy(uint8_t *dest, const uint8_t *src, size_t count) const
      noexcept override;
  uint8_t *set(uint8_t *dest, int ch, size_t count) const
      noexcept override;
};

void setMM(std::shared_ptr<MemoryManager> mm);
void setMM2(MemoryManager* mm);