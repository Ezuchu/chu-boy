#include "ppu.h"
#include "bus.h"
#include <cstdint>

Ppu::Ppu() {}

Ppu::~Ppu() {}

void Ppu::oamSearch() {
  this->state = OAMsearch;
  *LCDC |= 0x2;
  uint8_t up_limit;
  uint8_t down_limit;
  uint8_t size;
  if (*LCDC & 0x04) {
    up_limit = 0;
    down_limit = 160;
    size = 16;
  } else {
    up_limit = 8;
    down_limit = 168;
    size = 8;
  }

  object_type *obj = (object_type *)bus->Oam.get_address(oam_index);
  if (obj->y > up_limit && obj->y < down_limit && obj_index < 10) {
    if (*LY > obj->y && *LY <= obj->y - size) {
      objects[obj_index] = obj;
      obj_index++;
    }
  }
  oam_index += 0x04;
}

void Ppu::pixelTransfer() {}

void Ppu::hBlank() {}

void Ppu::vBlank() {}

void Ppu::connectBus(Bus *bus) {
  this->bus = bus;
  this->LCDC = bus->io.get_address(0x40);
  this->STAT = bus->io.get_address(0x41);
  this->SCY = bus->io.get_address(0x42);
  this->SCX = bus->io.get_address(0x43);
  this->LY = bus->io.get_address(0x44);
  this->LYC = bus->io.get_address(0x45);
  this->DMA = bus->io.get_address(0x46);
  this->BGP = bus->io.get_address(0x47);
  this->OBP0 = bus->io.get_address(0x48);
  this->OBP1 = bus->io.get_address(0x49);
  this->WY = bus->io.get_address(0x4A);
  this->WX = bus->io.get_address(0x4B);
}