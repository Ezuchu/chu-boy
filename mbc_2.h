#pragma once
#include "mbc.h"

class MBC_2 : public rom_controller {
private:
  uint8_t bank_number;

public:
  MBC_2(bool has_battery = false);
  ~MBC_2() override;
  void load_cartridge(Cartridge *cart) override;
  uint8_t read(uint16_t address) override;
  void write(uint16_t address, uint8_t data) override;
  void save_state() override;
};