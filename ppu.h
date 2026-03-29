#pragma once

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

  uint16_t oam_index = 0xFE00;
  uint8_t obj_index = 0;

  object_type *objects[10];

  Bus *bus = nullptr;

  void oamSearch();
  void pixelTransfer();
  void hBlank();
  void vBlank();

public:
  Ppu();
  ~Ppu();

  void connectBus(Bus *bus);

  void step();
};