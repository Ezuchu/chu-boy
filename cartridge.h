#pragma once

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

enum cart_type { ROM_ONLY = 0x0 };

class Cartridge {

  uint8_t *rom_data;

public:
  char game_title[16];
  bool cgb_flag;
  char license[2];
  uint8_t cartridge_type;
  uint8_t rom_size;
  uint8_t ram_size;

  Cartridge(std::string rom_name);
  ~Cartridge() { delete[] rom_data; };

  void output_rom_data();
  uint8_t *return_rom_data();
};