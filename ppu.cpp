#include "ppu.h"
#include "bus.h"
#include <cstdint>

Ppu::Ppu() {}

Ppu::~Ppu() {}

void Ppu::sort_objects_by_x() {}

void Ppu::oamSearch() {
  if (this->state != OAMsearch) {
    this->state = OAMsearch;
    *LCDC = (*LCDC & 0xFB) | 0x02;
    *STAT = (*STAT & 0xFB) | 0x02;
    //*STAT = (*STAT & 0xA4) | 0x20;

    // request interrupt
    //*IF |= 0x02;
  }
  this->state = OAMsearch;
  *LCDC = (*LCDC & 0xFB) | 0x02;
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

void Ppu::pixelTransfer() {
  // TODO: object pixel_search
  if (this->state != Pixeltransfer) {
    this->state = Pixeltransfer;
    *LCDC = (*LCDC & 0xFB) | 0x03;
    *STAT = (*STAT & 0xFB) | 0x03;
  }

  uint8_t bg_x = ((*SCX + lx) & 255) / 8;
  uint8_t bg_y = ((*SCY + *LY) & 255) / 8;

  uint16_t background_area = ((*LCDC & 0x08) == 0x08) ? 0x9C00 : 0x9800;
  uint16_t tile_data_area = ((*LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

  uint8_t bg_pixel_index =
      bus->read(background_area + (32 * (bg_y % 32)) + (bg_x % 32));

  uint16_t tile_data_address;
  if (tile_data_area == 0x8000) {
    tile_data_address = tile_data_area + (uint16_t)(16 * bg_pixel_index);
  } else {
    tile_data_address = 0x9000 + (int16_t)(16 * bg_pixel_index);
  }

  uint8_t bg_pixel_low = bus->read(tile_data_address + 2 * ((*SCY + *LY) % 8));
  uint8_t bg_pixel_high =
      bus->read(tile_data_address + 2 * ((*SCY + *LY) % 8) + 0x0001);

  uint8_t bg_pixel = ((bg_pixel_high >> (7 - ((*SCX + lx) % 8))) & 0x01) << 1 |
                     ((bg_pixel_low >> (7 - ((*SCX + lx) % 8))) & 0x01);

  bg_pixel = (*BGP >> (bg_pixel * 2)) & 0x03;

  this->vga->push_pixel(bg_pixel, lx, *LY);
}

void Ppu::hBlank() {
  if (this->state != HBlank) {
    this->state = HBlank;
    *LCDC = (*LCDC & 0xFB) | 0x00;
    *STAT = (*STAT & 0xFB) | 0x00;
    //*STAT = (*STAT & 0xA4) | 0x08;

    // request interrupt
    //*IF |= 0x02;
  }
}

void Ppu::vBlank() {
  if (this->state != VBlank) {
    this->state = VBlank;
    *LCDC = (*LCDC & 0xFB) | 0x01;
    *STAT = (*STAT & 0xFB) | 0x01;
    // *STAT = (*STAT & 0xA4) | 0x10;

    // request interrupt
    *IF |= 0x01;
    //*IF |= 0x02;
  }
}

void Ppu::connectBus(Bus *bus) {
  this->bus = bus;
  this->LCDC = bus->io.get_address(0x40);
  *LCDC = 0x91;
  this->STAT = bus->io.get_address(0x41);
  *STAT = 0x85;
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

  this->IF = bus->get_address(0xFF0F);
  this->IE = bus->get_address(0xFFFF);
}

void Ppu::connectVga(Vga *vga) { this->vga = vga; }

void Ppu::step(uint8_t cycles) {

  if ((*LCDC & 0x80) == 0x80) {
    act_cycles += cycles;
    while (act_cycles > 0) {
      if (*LY < 144) {
        if (cycle_counter < 80) {
          oamSearch();
          act_cycles -= 2;
          cycle_counter += 2;
        } else {
          if (cycle_counter < 252 && *LY < 144 && lx < 160) {
            pixelTransfer();
            act_cycles -= 1;
            cycle_counter += 1;
            lx++;
          } else if (lx >= 160 && cycle_counter < 456) {
            /*if (this->state == Pixeltransfer) {
              *LY = *LY + 1;
            }*/
            hBlank();
            act_cycles -= 4;
            cycle_counter += 4;
            lx++;
            if (cycle_counter >= 456) {
              oam_index = 0;
              *LY = *LY + 1;
              cycle_counter -= 456;
              lx = 0;
            }
          }
        }
      } else {
        if (cycle_counter < 456 && *LY < 154) {
          vBlank();
          act_cycles -= 4;
          cycle_counter += 4;
          if (cycle_counter >= 456) {
            *LY = *LY + 1;
            cycle_counter -= 456;
            if (*LY >= 154) {
              *LY = 0;
              vga->render();
            }
          }
        }
      }
    }
  }
}