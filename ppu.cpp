#include "ppu.h"
#include "bus.h"
#include <cstdint>

Ppu::Ppu() {}

Ppu::~Ppu() {}

void Ppu::sort_objects_by_x() {}

void Ppu::oamSearch() {
  if (this->state != OAMsearch) {
    this->state = OAMsearch;
    *LCDC = (*LCDC & 0xFC) | 0x02;
    *STAT = (*STAT & 0xFC) | 0x02;

    if ((*STAT & 0x20) == 0x20) {
      // request interrupt
      *IF |= 0x02;
    }
  }
  this->state = OAMsearch;
  *LCDC = (*LCDC & 0xFB) | 0x02;
  uint8_t up_limit;
  uint8_t down_limit;
  uint8_t size;
  if ((*LCDC & 0x04) == 0x04) {
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
    if (*LY >= obj->y - 16 && *LY < (obj->y - 16 + size)) {
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
    *LCDC = (*LCDC & 0xFC) | 0x03;
    *STAT = (*STAT & 0xFC) | 0x03;
    act_obj_index = 0;
  }
  obj = nullptr;
  if ((*LCDC & 0x02) == 0x02) {
    for (uint8_t i = 0; i < obj_index; i++) {
      if (objects[i]->x > 0 && objects[i]->x < 160) {
        if (lx >= objects[i]->x - 8 && objects[i]->x > lx) {
          if (obj != nullptr) {
            if (objects[i]->x < obj->x) {
              obj = objects[i];
            }
          } else {
            obj = objects[i];
          }
        }
      }
    }
  }
  if (obj != nullptr && ((obj->flags & 0x80) != 0x80)) {
    this->objectTransfer(obj);
    return;
  }

  // window enable in range?
  if (*LCDC & 0x20 && lx >= *WX && *LY >= *WY) {
    Ppu::windowTransfer();
    return;
  } else {
    Ppu::backgroundTransfer();
    return;
  }
}

void Ppu::objectTransfer(object_type *obj) {
  uint8_t obj_x = (lx - obj->x + 8) % 8;
  uint8_t obj_y = (*LY - (obj->y - 16)) % ((*LCDC & 0x04) ? 16 : 8);
  uint16_t obj_tile = obj->tile;
  uint8_t obj_flags = obj->flags;

  uint8_t x_flip = (obj_flags & 0x20) ? 1 : 0;
  uint8_t y_flip = (obj_flags & 0x40) ? 1 : 0;

  uint16_t tile_data_address = 0x8000 + (uint16_t)(16 * obj_tile);

  uint8_t row_num =
      !y_flip ? obj_y
              : (((*LCDC & 0x04) ? 15 : 7) - obj_y) % ((*LCDC & 0x04) ? 16 : 8);

  uint8_t obj_pixel_low = bus->read(tile_data_address + (2 * row_num));
  uint8_t obj_pixel_high =
      bus->read(tile_data_address + (2 * row_num) + 0x0001);

  uint8_t pixel_num = x_flip ? obj_x : 7 - obj_x;

  uint8_t obj_pixel = ((obj_pixel_high >> (pixel_num)) & 0x01) << 1 |
                      ((obj_pixel_low >> (pixel_num)) & 0x01);

  if ((obj_flags & 0x10) == 0x10) {
    obj_pixel = (*OBP1 >> (obj_pixel * 2)) & 0x03;
  } else {
    obj_pixel = (*OBP0 >> (obj_pixel * 2)) & 0x03;
  }
  this->vga->push_pixel(obj_pixel, lx, *LY);
}

void Ppu::windowTransfer() {
  uint8_t bg_x = (((lx + 7) - *WX) & 255) / 8;
  uint8_t bg_y = ((*LY - *WY) & 255) / 8;

  uint16_t window_area = ((*LCDC & 0x40) == 0x40) ? 0x9C00 : 0x9800;
  uint16_t tile_data_area = ((*LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

  uint8_t bg_pixel_index =
      bus->read(window_area + (32 * (bg_y % 32)) + (bg_x % 32));

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

  if (bg_pixel == 0 && obj != nullptr && (obj->flags & 0x80) == 0x80) {
    this->objectTransfer(obj);
    return;
  }

  bg_pixel = (*BGP >> (bg_pixel * 2)) & 0x03;

  this->vga->push_pixel(bg_pixel, lx, *LY);
}

void Ppu::backgroundTransfer() {
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

  if (bg_pixel == 0 && obj != nullptr && (obj->flags & 0x80) == 0x80) {
    this->objectTransfer(obj);
    return;
  }

  bg_pixel = (*BGP >> (bg_pixel * 2)) & 0x03;

  this->vga->push_pixel(bg_pixel, lx, *LY);
}

void Ppu::hBlank() {
  if (this->state != HBlank) {
    this->state = HBlank;
    *LCDC = (*LCDC & 0xFC) | 0x00;
    *STAT = (*STAT & 0xFC) | 0x00;
    this->obj_index = 0;

    if ((*STAT & 0x08) == 0x08) {
      // request interrupt
      *IF |= 0x02;
    }
    //*STAT = (*STAT & 0xA4) | 0x08;

    // request interrupt
    //*IF |= 0x02;
  }
}

void Ppu::vBlank() {
  if (this->state != VBlank) {
    this->state = VBlank;
    *LCDC = (*LCDC & 0xFC) | 0x01;
    *STAT = (*STAT & 0xFC) | 0x01;

    if ((*STAT & 0x10) == 0x10) {
      // request interrupt
      *IF |= 0x02;
    }

    // *STAT = (*STAT & 0xA4) | 0x10;

    // request interrupt
    *IF |= 0x01;
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

              if (*LY == *LYC) {
                *STAT |= 0x20;
                if ((*STAT & 0x40) == 0x40) {
                  *IF |= 0x02;
                }
              }
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
            if (*LY == *LYC) {
              *STAT |= 0x20;
              if ((*STAT & 0x40) == 0x40) {
                *IF |= 0x02;
              }
            }
          }
        }
      }
    }
  }
}