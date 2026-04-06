#pragma once

#include "vga.h"
#include <cstdint>

class Bus;

enum PpuState { OAMsearch, Pixeltransfer, HBlank, VBlank };

class Ppu {
  struct object_type {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t flags;
  };

  PpuState state;

  uint8_t *LCDC;
  uint8_t *STAT;
  uint8_t *SCY;
  uint8_t *SCX;
  uint8_t *LY;
  uint8_t *LYC;
  uint8_t *DMA;
  uint8_t *BGP;
  uint8_t *OBP0;
  uint8_t *OBP1;
  uint8_t *WY;
  uint8_t *WX;

  uint8_t *IF;
  uint8_t *IE;

  uint8_t lx = 0;
  int16_t cycle_counter = 0;
  int8_t act_cycles = 0;

  uint16_t oam_index = 0xFE00;
  uint8_t obj_index = 0;

  object_type *objects[10];

  Bus *bus = nullptr;
  Vga *vga = nullptr;

  void sort_objects_by_x();

  void oamSearch();
  void pixelTransfer();
  void hBlank();
  void vBlank();

public:
  Ppu();
  ~Ppu();

  void connectBus(Bus *bus);
  void connectVga(Vga *vga);

  void step(uint8_t cycles);
};