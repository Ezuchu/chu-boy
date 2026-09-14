#pragma once

#include "vga.h"
#include <cstdint>

class Bus;

enum PpuState { OFF, OAMsearch, Pixeltransfer, HBlank, VBlank };

class Ppu {
  struct object_type {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t flags;
  };

  struct bg_tile {
    uint8_t tile_id;
    uint8_t attributes;
    uint8_t low_byte;
    uint8_t high_byte;
  };

  struct BG_pixel_type {
    uint8_t color;
    uint8_t palette;
    uint8_t bg_priority;
  };

  struct OBJ_pixel_type {
    uint8_t color;
    uint8_t palette;
    uint8_t obj_priority;
  };

  enum Fetcher_State { FetchTileId, FetchLowByte, FetchHighByte, sleep };
  Fetcher_State fetcher_state = FetchTileId;

  enum FIFO_State { FirstBG, BgRender, Sprite, FirstW };
  FIFO_State fifo_state = FirstBG;

  bg_tile current_bg_tile;

  BG_pixel_type bg_fifo[8];
  OBJ_pixel_type obj_fifo[8];

  int bg_fifo_index = 0;
  int obj_fifo_index = 0;

  bool ppu_was_on;

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

  uint8_t *BGPI;
  uint8_t *OBPI;

  uint8_t *WY;
  uint8_t *WX;

  uint8_t *IF;
  uint8_t *IE;

  uint8_t lx = 0;
  int16_t cycle_counter = 0;
  int8_t act_cycles = 0;

  uint16_t oam_index = 0xFE00;
  uint8_t obj_index = 0;
  uint8_t act_obj_index = 0;

  object_type *objects[10];
  object_type *obj = nullptr;

  uint8_t bg_attributes = 0x00;

  uint8_t pixel_to_draw = 0;

  Bus *bus = nullptr;
  Vga *vga = nullptr;

  void sort_objects_by_x();

  void handle_enable_disable();

  void oamSearch();
  uint8_t getObjPixel(object_type *obj);
  void pixelTransfer();
  uint8_t getWinPixel();
  uint8_t getBgPixel();
  void hBlank();
  void vBlank();

  void getBgTile();

  int fetcher_cycles = 0;

  uint8_t bg_pixel_buffer[8];
  uint8_t buffer_index = 8;

public:
  PpuState state;
  bool CGB = false;
  Ppu();
  ~Ppu();

  void connectBus(Bus *bus);
  void connectVga(Vga *vga);

  void restartToDMG();

  void step(uint8_t cycles);
};