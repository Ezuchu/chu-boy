#pragma once
#include "mbc.h"

class MBC_3 : public rom_controller {
private:
  uint8_t bank_number;

  bool clock = false;
  uint8_t rtc_regs[5];

  bool rtc_enable = false;
  bool latch = false;

public:
  MBC_3(bool has_battery = false, bool has_clock = false);
  ~MBC_3() override;
  void load_cartridge(Cartridge *cart) override;
  uint8_t read(uint16_t address) override;
  void write(uint16_t address, uint8_t data) override;
  void save_state() override;
};