#pragma once

#include <cstdint>

class Bus;

class DMA {
  uint16_t start_address;
  uint16_t end_address;

  uint16_t act_OAM;

  uint16_t act_address;

  uint16_t cycles;

  Bus *bus;

public:
  bool state;

  void dma_start(uint8_t start);
  void dma_step();
  uint8_t read();
  void write(uint8_t data);
  DMA(Bus *bus);
};