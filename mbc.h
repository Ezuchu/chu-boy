#pragma once

#include "cartridge.h"
#include <cstdint>
#include <string>

const int ram_ref[] = {1, 1, 0x2000, 0x8000, 0x20000, 0x10000};

class rom_controller {
protected:
  uint8_t rom_size;
  uint8_t ram_size;

  uint8_t *rom_bank;
  uint8_t *ram_bank;

  uint8_t ram_bank_number;
  bool ram_enable;
  bool battery;

  std::string filename;
  std::string savename;

public:
  virtual ~rom_controller() { std::cout << "" << std::endl; }
  virtual void load_cartridge(Cartridge *cart) = 0;
  virtual uint8_t read(uint16_t address) = 0;
  virtual void write(uint16_t address, uint8_t data) = 0;
  virtual void save_state() = 0;
};

class mbc_factory {
public:
  static rom_controller *create_mbc(Cartridge *cart);
};