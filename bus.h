#pragma once

#include "cpu.h"
#include "dma.h"
#include "mbc.h"
#include "memory.h"
#include "ppu.h"
#include "apu.h"
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

  Cpu cpu;
  Ppu ppu;
  Apu apu;
  rom_controller *rom = nullptr;
  Vga *vga = nullptr;
  DMA dma;

  uint8_t *tima = nullptr;
  uint8_t *tma = nullptr;
  uint8_t *tac = nullptr;
  uint8_t *div = nullptr;

  uint8_t *IF = nullptr;

  uint16_t timer_counter = 0;
  uint16_t div_counter = 0;

  Bus();
  ~Bus();

  void write(uint8_t data, uint16_t address);
  uint8_t read(uint16_t address);
  uint8_t *get_address(uint16_t address);
  void clock();
  void clock(uint8_t cycles);

  void timer_clock(uint8_t cycles);
};