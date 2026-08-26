#include "mbc_3.h"
#include <cstring>

MBC_3::MBC_3(bool has_battery, bool has_clock) {
  this->battery = has_battery;
  this->clock = has_clock;
}

MBC_3::~MBC_3() {
  std::cout << "in destructor" << std::endl;
  save_state();
  delete[] ram_bank;
}

void MBC_3::load_cartridge(Cartridge *cart) {

  this->filename = cart->filename;
  this->savename = filename.substr(0, filename.find_last_of(".")) + ".sav";

  this->rom_bank = cart->return_rom_data();
  this->rom_size = rom_bank[0x0148];
  this->ram_size = rom_bank[0x0149];
  this->bank_number = 0x01;

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

void MBC_3::save_state() {
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

void MBC_3::write(uint16_t address, uint8_t data) {
  static const int rom_ref[] = {0, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F};
  if (address <= 0x1FFF) {
    if (data == 0xA) {
      ram_enable = true;
      if (clock) {
        rtc_enable = true;
      }
    } else {
      ram_enable = false;
      rtc_enable = false;
    }
  } else if (address <= 0x3FFF) {
    bank_number = (data & 0x7F) == 0 ? 1 : (data & 0x7F) & rom_ref[rom_size];
  } else if (address <= 0x5FFF) {
    ram_bank_number = data & 0x0C;
  } else if (address <= 0x7FFF) {
    // TODO: latch
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_bank_number < 0x08 || !clock) {
      if (ram_enable && ram_size > 1) {
        ram_bank[address - 0xA000 + (0x2000 * (ram_bank_number & 0x03))] = data;
      }
    } else {
      // TODO: write to rtc
    }
  }
}

uint8_t MBC_3::read(uint16_t address) {
  if (address < 0x4000) {
    return rom_bank[address & 0x3FFF];
  } else if (address < 0x8000) {
    return rom_bank[(address & 0x3FFF) + (bank_number * 0x4000)];
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_bank_number < 0x08 || !clock) {
      if (ram_enable && ram_size > 1) {
        return ram_bank[address - 0xA000 + (0x2000 * ram_bank_number)];
      }
    }
    return 0xFF;
  } else {
    return 0;
  }
}