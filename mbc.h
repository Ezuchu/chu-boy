#pragma once

#include "cartridge.h"
#include <cstdint>
class rom_controller {
protected:
public:
  virtual void load_cartridge(Cartridge *cart) = 0;
  virtual uint8_t read(uint16_t address) = 0;
};

class MBC : public rom_controller {
public:
  virtual void switch_bank() = 0;
};