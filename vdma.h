#pragma once

#include <cstdint>

class Bus;

class VDMA {
  uint16_t source_address;
  uint16_t dest_address;

  uint16_t act_VRAM;

  uint16_t act_address;

  uint16_t cycles;

  uint8_t *hdma5;

  int length;
  int bytes_written;

  Bus *bus;

public:
  int state;
  VDMA(Bus *bus);
  void vdma_start(uint8_t mode, uint8_t length);
  void vdma_step(uint8_t bytes);
  uint8_t read();
  void write(uint8_t data);
};
