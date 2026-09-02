#include "mbc_3.h"
#include <cstring>
#include <ctime>

const std::time_t BASE_TIME = 946684800;

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
      if (clock) {
        rom_save.read(reinterpret_cast<char *>(rtc_regs), sizeof(rtc_regs));
        if (rom_save) {
          rom_save.read(reinterpret_cast<char *>(&last_time_register),
                        sizeof(uint32_t));
          if (rom_save) {
            rom_save.read(reinterpret_cast<char *>(latch_regs),
                          sizeof(latch_regs));
          }
        } else {
          initialize_rtc();
        }
        std::cout << (int)rtc_regs[0] << " " << (int)rtc_regs[1] << " "
                  << (int)rtc_regs[2] << " " << (int)rtc_regs[3] << " "
                  << (int)rtc_regs[4] << std::endl;
        /*update_rtc();
        std::cout << (int)rtc_regs[0] << " " << (int)rtc_regs[1] << " "
                  << (int)rtc_regs[2] << " " << (int)rtc_regs[3] << " "
                  << (int)rtc_regs[4] << std::endl;*/
      }

      rom_save.close();
    } else {
      memset(this->ram_bank, 0x00, ram_ref[this->ram_size]);
      if (clock) {
        initialize_rtc();
      }
    }
  }
}

// initialize rtc regs with system date in seconds
void MBC_3::initialize_rtc() {
  const std::time_t BASE_TIME = 946684800; // 2000-01-01 00:00:00 UTC
  std::time_t act_time = std::time(0);
  std::time_t diff = act_time - BASE_TIME;
  last_time_register = act_time;

  uint32_t days = diff / 86400;
  uint32_t remaining = diff % 86400;

  uint8_t seconds = remaining % 60;
  uint8_t minutes = (remaining / 60) % 60;
  uint8_t hours = (remaining / 3600) % 24;

  uint8_t day_low = days & 0xFF;
  uint8_t day_high = (days >> 8) & 0x01;

  uint8_t carry = ((days >> 9) & 0x01) > 0 ? 1 : 0;

  rtc_regs[0] = seconds;
  rtc_regs[1] = minutes;
  rtc_regs[2] = hours;
  rtc_regs[3] = day_low;
  rtc_regs[4] = day_high | (carry << 7);
}

void MBC_3::update_rtc() {

  uint32_t diff = (std::time(0) - BASE_TIME) - last_time_register;
  last_time_register = std::time(0) - BASE_TIME;
  /*uint32_t old_time = rtc_to_seconds();
  uint32_t diff = last_time_register - old_time;*/

  seconds_to_rtc(rtc_to_seconds() + diff);
}

void MBC_3::seconds_to_rtc(uint32_t seconds) {
  uint32_t days = seconds / 86400;
  uint32_t remaining = seconds % 86400;

  uint8_t secs = remaining % 60;
  uint8_t mins = (remaining / 60) % 60;
  uint8_t hrs = (remaining / 3600) % 24;

  uint8_t day_low;
  uint8_t day_high = 0;

  if (days > 511) {
    day_high |= 0x80;
    uint16_t wrapped_days = days % 512;
    day_low = wrapped_days & 0xFF;
    day_high |= (wrapped_days >> 8) & 0x01;
  } else {
    day_low = days & 0xFF;
    day_high |= (days >> 8) & 0x01;
  }

  rtc_regs[0] = secs;
  rtc_regs[1] = mins;
  rtc_regs[2] = hrs;
  rtc_regs[3] = day_low;
  rtc_regs[4] = day_high;
}

uint32_t MBC_3::rtc_to_seconds() {
  uint16_t days = rtc_regs[3] | ((rtc_regs[4] & 0x01) << 8);

  // Añadir 512 si el carry está activo
  days += ((rtc_regs[4] >> 7) == 1) ? 512 : 0;

  return rtc_regs[0] + (rtc_regs[1] * 60) + (rtc_regs[2] * 3600) +
         (days * 86400);
}

void MBC_3::save_state() {
  if (battery && ram_ref[this->ram_size] > 1) {
    std::ofstream rom_save;
    rom_save.open(savename, std::ofstream::binary);
    if (rom_save.is_open()) {
      std::cout << "open" << std::endl;
      rom_save.write(reinterpret_cast<char *>(ram_bank),
                     ram_ref[this->ram_size]);
      if (clock) {
        rom_save.write(reinterpret_cast<char *>(rtc_regs), sizeof(rtc_regs));
        rom_save.write(reinterpret_cast<char *>(&last_time_register),
                       sizeof(uint32_t));
        rom_save.write(reinterpret_cast<char *>(latch_regs),
                       sizeof(latch_regs));
      }
      rom_save.close();
      std::cout << "saved" << std::endl;
    }
  }
}

void MBC_3::write(uint16_t address, uint8_t data) {
  static const int rom_ref[] = {0, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
  if (address <= 0x1FFF) {
    if ((data & 0x0F) == 0x0A) {
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
    ram_bank_number = data & 0x0F;
    if (ram_bank_number > 0x0C) {
      ram_bank_number -= 0x0C;
    }
  } else if (address <= 0x7FFF) {
    if (!latch && (data & 0x01)) {
      latch = true;
      update_rtc();
      for (int i = 0; i < 5; i++) {
        latch_regs[i] = rtc_regs[i];
      }
    } else if (latch && !(data & 0x01)) {
      latch = false;
    }
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_bank_number < 0x08 || !clock) {
      if (ram_enable && ram_size > 1) {
        ram_bank[address - 0xA000 + (0x2000 * (ram_bank_number & 0x03))] = data;
      }
    } else {
      if (ram_bank_number >= 0x08 && clock) {
        if (rtc_enable) {
          update_rtc();
          switch (ram_bank_number) {
          case 0x08:
          case 0x09:
            data %= 60;
            break;
          case 0x0A:
            data %= 24;
            break;
          default:
            break;
          }
          rtc_regs[(ram_bank_number - 0x08) % 5] = data & 0xFF;
        }
      }
    }
  }
}

uint8_t MBC_3::read(uint16_t address) {
  if (address < 0x4000) {
    return rom_bank[address & 0x3FFF];
  } else if (address < 0x8000) {
    if (bank_number == 0) {
      bank_number = 1;
    }
    return rom_bank[(address & 0x3FFF) + (bank_number * 0x4000)];
  } else if (address >= 0xA000 && address <= 0xBFFF) {
    if (ram_bank_number < 0x08 || !clock) {
      if (ram_enable && ram_size > 1) {
        return ram_bank[address - 0xA000 + (0x2000 * (ram_bank_number & 0x03))];
      }
    } else if (clock) {
      if (rtc_enable) {
        if (latch) {
          return latch_regs[(ram_bank_number - 0x08) % 5];
        }
        update_rtc();
        return rtc_regs[(ram_bank_number - 0x08) % 5];
      }
    }
    return 0xFF;
  } else {
    return 0;
  }
}