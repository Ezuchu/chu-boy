#include "no_mbc.h"
#include <cstdint>

No_mbc_controller::No_mbc_controller() {}
No_mbc_controller::~No_mbc_controller() {}

void No_mbc_controller::load_cartridge(Cartridge *cart) {
  this->cart = cart;
  this->data = cart->return_rom_data();
}

uint8_t No_mbc_controller::read(uint16_t address) { return *(data + address); }
