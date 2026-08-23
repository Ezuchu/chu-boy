#pragma once

#include "cartridge.h"
#include "mbc.h"

class No_mbc_controller : public rom_controller {
  Cartridge *cart;
  uint8_t *data;

public:
  No_mbc_controller();
  ~No_mbc_controller();

  void load_cartridge(Cartridge *cart) override;
  uint8_t read(uint16_t address) override;
  void write(uint16_t address, uint8_t data) override;
  void save_state() override {};
};