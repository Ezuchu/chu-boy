#pragma once

#include "apu.h"
#include "cpu.h"
#include "dma.h"
#include "mbc.h"
#include "memory.h"
#include "ppu.h"
#include <array>
#include <cstddef>
#include <cstdint>

class rom_controller;
class Vga;

class Bus {
public:
  Memory ram;
  Memory Vram;
  Memory Oam;
  Memory io;
  Memory hram;

  Memory Cram;

  Cpu cpu;
  Ppu ppu;
  Apu apu;
  rom_controller *rom = nullptr;
  Vga *vga = nullptr;
  DMA dma;

  bool CGB = false;

  uint8_t *tima = nullptr;
  uint8_t *tma = nullptr;
  uint8_t *tac = nullptr;
  uint8_t *div = nullptr;

  uint8_t *IF = nullptr;

  uint8_t *BGPI = nullptr;
  uint8_t *BGPD = nullptr;
  uint8_t *OBPI = nullptr;
  uint8_t *OBPD = nullptr;

  uint16_t timer_counter = 0;
  uint16_t div_counter = 0;

  uint32_t frame_counter = 0;
  uint32_t save_frame_interval = 600; // 60 frames * seconds

  Bus(bool is_cgb = false);
  ~Bus();

  void write(uint8_t data, uint16_t address);
  uint8_t read(uint16_t address, bool is_cpu = false);
  uint8_t *get_address(uint16_t address);
  void clock();
  void clock(uint8_t cycles);

  void write_bg_cram(uint8_t data);
  void write_ob_cram(uint8_t data);

  void timer_clock(uint8_t cycles);

  void frame_completed();
};