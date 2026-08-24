#include "mbc.h"
#include "mbc_1.h"
#include "no_mbc.h"

rom_controller *mbc_factory::create_mbc(Cartridge *cart) {
  switch (cart->cartridge_type) {
  case 0x00:
    return new No_mbc_controller();
  case 0x01:
  case 0x02:
    return new MBC_1();
  case 0x03:
  case 0x04:
    return new MBC_1(true);
  default:
    return nullptr;
  }
}
