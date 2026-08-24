#include "mbc_2.h"
#include <cstring>

MBC_2::MBC_2(bool has_battery) { this->battery = has_battery; }

MBC_2::~MBC_2() {
  std::cout << "in destructor" << std::endl;
  save_state();
  delete[] ram_bank;
}

void MBC_2::load_cartridge(Cartridge *cart) {

  this->filename = cart->filename;
  this->savename = filename.substr(0, filename.find_last_of(".")) + ".sav";

  this->rom_bank = cart->return_rom_data();
  this->rom_size = rom_bank[0x0148];
  this->bank_number = 0x01;

  std::cout << (int)rom_size << " " << (int)ram_size << std::endl;

  this->ram_bank = new uint8_t[0x200]; // 512 bytes
  this->ram_bank_number = 0;
  this->ram_enable = false;

  if (battery) {
    std::ifstream rom_save;
    rom_save.open(savename, std::ifstream::binary);
    if (rom_save.is_open()) {
      rom_save.read(reinterpret_cast<char *>(ram_bank), 0x200);
      rom_save.close();
    } else {
      memset(this->ram_bank, 0x00, 0x200);
    }
  }
}

void MBC_2::save_state() {
  if (battery) {
    std::ofstream rom_save;
    rom_save.open(savename, std::ofstream::binary);
    if (rom_save.is_open()) {
      std::cout << "open" << std::endl;
      rom_save.write(reinterpret_cast<char *>(ram_bank), 0x200);
      rom_save.close();
      std::cout << "saved" << std::endl;
    }
  }
}

void MBC_2::write(uint16_t address, uint8_t data) {
  static const int rom_ref[] = {0, 0x3, 0x7, 0xF, 0x1F, 0x1F, 0x1F};
  if (address <= 0x3FFF) {
    uint8_t high_address = uint8_t(address >> 8);
    if (high_address & 0x01) {
      // switch rom bank
      bank_number = (data & 0x0F) == 0x00 ? 0x01 : data & 0x0F;
    } else {
      // enable-disable ram
      if (data == 0x0A) {
        ram_enable = true;
      } else {
        ram_enable = false;
      }
    }
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_enable) {
      ram_bank[(address - 0xA000) & 0x01FF] = data & 0x0F;
    }
  }
}

uint8_t MBC_2::read(uint16_t address) {
  if (address < 0x4000) {
    // rom bank 0
    return rom_bank[address & 0x3FFF];
  } else if (address < 0x8000) {
    // rom bank 1
    return rom_bank[(address & 0x3FFF) + (bank_number * 0x4000)];
  } else if (address >= 0xA000) {
    if (ram_enable) {
      // ram reading (returns only 4 lower bits)
      if (address <= 0xA1FF) {
        return (ram_bank[address - 0xA000] & 0x0F);
      } else {
        return (ram_bank[(address - 0xA000) & 0x01FF] & 0x0F);
      }
    } else {
      return 0x0F;
    }
  } else {
    return 0;
  }
}