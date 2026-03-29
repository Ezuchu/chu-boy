#pragma once

#include "cpu.h"
#include "mbc.h"
#include "memory.h"
#include "ppu.h"
#include <array>
#include <cstdint>

class rom_controller;

class Bus {
public:
  Memory ram;
  Memory Vram;
  Memory Oam;
  Memory io;
  Memory hram;

  Cpu cpu;
  Ppu ppu;
  rom_controller *rom = nullptr;

  Bus();
  ~Bus();

  void write(uint8_t data, uint16_t address);
  uint8_t read(uint16_t address);
  void clock();
};