#pragma once

#include "mbc.h"

class MBC_5 : public rom_controller {
private:
  uint8_t bank1;
  uint8_t bank2;

  uint8_t bank_mode = 0;

public:
  MBC_5(bool has_battery = false);
  ~MBC_5() override;
  void load_cartridge(Cartridge *cart) override;
  uint8_t read(uint16_t address) override;
  void write(uint16_t address, uint8_t data) override;
  void save_state() override;
};