#pragma once

#include "cartridge.h"
#include <cstdint>
#include <string>

const int ram_ref[] = {1, 1, 0x2000, 0x8000, 0x20000, 0x10000};

class rom_controller {
protected:
  uint8_t rom_size;
  uint8_t ram_size;

  std::string filename;
  std::string savename;

public:
  virtual ~rom_controller() = default;
  virtual void load_cartridge(Cartridge *cart) = 0;
  virtual uint8_t read(uint16_t address) = 0;
  virtual void write(uint16_t address, uint8_t data) = 0;
};

class MBC_1 : public rom_controller {
private:
  uint8_t *rom_bank;
  uint8_t *ram_bank;

  bool ram_enable;

  uint8_t bank1;
  uint8_t bank2;
  uint8_t ram_bank_number;

  bool battery;

  uint8_t bank_mode = 0;

public:
  MBC_1(bool has_battery = false);
  ~MBC_1();
  void load_cartridge(Cartridge *cart) override;
  uint8_t read(uint16_t address) override;
  void write(uint16_t address, uint8_t data) override;
};

class mbc_factory {
public:
  static rom_controller *create_mbc(Cartridge *cart);
};