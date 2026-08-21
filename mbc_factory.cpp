#include "mbc.h"
#include "no_mbc.h"

rom_controller *mbc_factory::create_mbc(Cartridge *cart) {
  switch (cart->cartridge_type) {
  case 0x0:
    return new No_mbc_controller();
  case 0x1:
  case 0x2:
  case 0x3:
  case 0x4:
    return new MBC_1();
  default:
    return nullptr;
  }
}
