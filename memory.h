#pragma once

#include <cstdint>
class Memory {
  uint8_t *data;
  uint16_t size;

public:
  Memory(uint16_t size);
  ~Memory();
  uint8_t read(uint16_t address);
  void write(uint8_t data, uint16_t address);
};