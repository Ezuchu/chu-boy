#pragma once

#include "ppu.h"
#include <cstdint>

class Bus;

class VDMA {
  PpuState ppu_prev_state;
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
  int vdma_step();
  uint8_t read_vdma();
  void write_source();
  void write_hdm1();
  void write_hdm2();
  void write_hdm3();
  void write_hdm4();
  void write_dest();
  uint8_t read();
  void write(uint8_t data);
};
