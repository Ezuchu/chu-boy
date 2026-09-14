#include "mbc_5.h"
#include <cstring>

MBC_5::MBC_5(bool has_battery) { this->battery = has_battery; }

MBC_5::~MBC_5() {
  std::cout << "in destructor" << std::endl;
  save_state();
  delete[] ram_bank;
}

void MBC_5::load_cartridge(Cartridge *cart) {

  this->filename = cart->filename;
  this->savename = filename.substr(0, filename.find_last_of(".")) + ".sav";

  this->rom_bank = cart->return_rom_data();
  this->rom_size = rom_bank[0x0148];
  this->ram_size = rom_bank[0x0149];
  this->bank1 = 1;
  this->bank2 = 0;

  std::cout << (int)rom_size << " " << (int)ram_size << std::endl;

  this->ram_bank = new uint8_t[ram_ref[this->ram_size]];
  this->ram_bank_number = 0;
  this->ram_enable = false;

  if (ram_ref[this->ram_size] > 1) {
    std::ifstream rom_save;
    rom_save.open(savename, std::ifstream::binary);
    if (battery && rom_save.is_open()) {
      rom_save.read(reinterpret_cast<char *>(ram_bank),
                    ram_ref[this->ram_size]);
      rom_save.close();
    } else {
      memset(this->ram_bank, 0x00, ram_ref[this->ram_size]);
    }
  }
}

void MBC_5::save_state() {
  if (battery && ram_ref[this->ram_size] > 1) {
    std::ofstream rom_save;
    rom_save.open(savename, std::ofstream::binary);
    if (rom_save.is_open()) {
      std::cout << "open" << std::endl;
      rom_save.write(reinterpret_cast<char *>(ram_bank),
                     ram_ref[this->ram_size]);
      rom_save.close();
      std::cout << "saved" << std::endl;
    }
  }
}

void MBC_5::write(uint16_t address, uint8_t data) {
  static const int rom_ref[] = {0, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0xFF};
  static const int ram_size_ref[] = {0x0, 0x0, 0x01, 0x03, 0x0F, 0x07};
  if (address <= 0x1FFF) {
    if (data == 0xA) {
      ram_enable = true;
    } else {
      ram_enable = false;
    }
  } else if (address <= 0x2FFF) {
    bank1 = (data & rom_ref[rom_size]);
  } else if (address <= 0x3FFF) {
    if (rom_size > 7) {
      bank2 = (data & 0x01);
    } else {
      bank1 = (data & rom_ref[rom_size]);
    }

  } else if (address <= 0x5FFF) {
    if (ram_enable && ram_size > 1) {
      ram_bank_number = data & ram_size_ref[ram_size];
    }
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_enable && ram_size > 1) {
      ram_bank[address - 0xA000 + (0x2000 * ram_bank_number)] = data;
    }
  }
}

uint8_t MBC_5::read(uint16_t address) {
  if (address < 0x4000) {
    return rom_bank[address & 0x3FFF];
  } else if (address < 0x8000) {
    uint16_t bank = (bank1 | (bank2 << 8));
    return rom_bank[(address & 0x3FFF) + (bank * 0x4000)];
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_enable && ram_size > 1) {
      return ram_bank[address - 0xA000 + (0x2000 * ram_bank_number)];
    }
    return 0xFF;
  } else {
    return 0;
  }
}
