#pragma once

#include "cpu.h"
#include "mbc.h"
#include "memory.h"
#include <array>
#include <cstdint>

class rom_controller;
class Ppu;

class Bus {

  Memory ram;
  Memory Vram;

public:
  Cpu cpu;
  rom_controller *rom = nullptr;

  Bus();
  ~Bus();

  void write(uint8_t data, uint16_t address);
  uint8_t read(uint16_t address);
  void clock();
};