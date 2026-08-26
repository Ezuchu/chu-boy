#include "mbc.h"
#include "mbc_1.h"
#include "mbc_2.h"
#include "mbc_3.h"
#include "no_mbc.h"

rom_controller *mbc_factory::create_mbc(Cartridge *cart) {
  // change mbc5 to mbc3
  /*if (cart->cartridge_type == 27) {
    cart->cartridge_type = 0x13;
  }*/

  switch (cart->cartridge_type) {
  case 0x00:
    return new No_mbc_controller();
  case 0x01:
  case 0x02:
    return new MBC_1();
  case 0x03:
  case 0x04:
    return new MBC_1(true);
  case 0x05:
    return new MBC_2();
  case 0x06:
    return new MBC_2(true);

  case 0x0F:
  case 0x10:
    return new MBC_3(true, true);
  case 0x11:
  case 0x12:
    return new MBC_3();
  case 0x13:
    return new MBC_3(true);
  default:
    return nullptr;
  }
}
