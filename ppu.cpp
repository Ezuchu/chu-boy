#include "ppu.h"
#include "bus.h"
#include <cstdint>

Ppu::Ppu() { ppu_was_on = true; }

Ppu::~Ppu() {}

void Ppu::sort_objects_by_x() {}

void Ppu::oamSearch() {
  if (this->state != OAMsearch) {
    this->state = OAMsearch;
    /*static int anterior = 256;
    if (((int)*LCDC) != anterior) {
      std::cout << std::hex << (int)*LCDC << std::endl;
      anterior = (int)*LCDC;
    }*/

    *STAT = (*STAT & 0xFC) | 0x02;
    if ((*STAT & 0x20) == 0x20) {
      // request interrupt
      *IF |= 0x02;
    }
  }

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
    int ypos = (int)obj->y - 16;
    if (*LY >= ypos && *LY < (ypos + size)) {
      objects[obj_index] = obj;
      obj_index++;
    }
  }
  oam_index += 0x04;
}

void Ppu::getWinTile() {
  current_bg_tile = {0, 0, 0, 0};
  if (!CGB && !(*LCDC & 0x01)) {
    for (int i = 0; i < 8; i++) {
      bg_fifo.push({0, 0, 0});
    }
    return;
  }
  uint16_t bg_x = fx % 32;
  uint16_t bg_y = (*LY - *WY) & 0xFF;
  uint16_t tile_y = (bg_y / 8) % 32;

  uint16_t window_area = ((*LCDC & 0x40) == 0x40) ? 0x9C00 : 0x9800;
  uint16_t tile_data_area = ((*LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

  uint16_t map_address = window_area + (32 * (tile_y)) + (bg_x);

  uint8_t bg_pixel_index = bus->Vram.read(map_address - 0x8000);

  uint16_t tile_index;
  if (tile_data_area == 0x8000) {
    tile_index = tile_data_area + (uint16_t)(16 * bg_pixel_index);
  } else {
    int8_t signed_index = (int8_t)bg_pixel_index;
    tile_index = 0x9000 + signed_index * 16;
  }

  if (CGB) {
    current_bg_tile.attributes = bus->Vram.read(map_address - 0x8000 + 0x2000);
  }
  uint8_t bank = (current_bg_tile.attributes >> 3) & 0x01;
  int row = (current_bg_tile.attributes >> 6) & 0x01 ? 7 - ((bg_y) % 8)
                                                     : ((bg_y) % 8);
  current_bg_tile.tile_id = tile_index + (2 * row) + (bank * 0x2000);

  current_bg_tile.low_byte = bus->Vram.read((current_bg_tile.tile_id) - 0x8000);

  current_bg_tile.high_byte =
      bus->Vram.read((current_bg_tile.tile_id + 0x0001) - 0x8000);

  for (int i = 0; i < 8; i++) {
    uint8_t x_flip = (current_bg_tile.attributes >> 5) & 0x01;

    uint8_t low_color_bit;
    uint8_t high_color_bit;
    if (x_flip) {
      low_color_bit = (current_bg_tile.low_byte >> (i)) & 0x01;
      high_color_bit = ((current_bg_tile.high_byte >> (i)) & 0x01) << 1;
    } else {
      low_color_bit = (current_bg_tile.low_byte >> (7 - i)) & 0x01;
      high_color_bit = ((current_bg_tile.high_byte >> (7 - i)) & 0x01) << 1;
    }
    uint8_t color = low_color_bit | high_color_bit;

    bg_fifo.push({color, (uint8_t)(current_bg_tile.attributes & 0x07), 0});
  }
}

void Ppu::getBgTile() {
  current_bg_tile = {0, 0, 0, 0};
  if (!CGB && !(*LCDC & 0x01)) {
    for (int i = 0; i < 8; i++) {
      bg_fifo.push({0, 0, 0});
    }
    return;
  }
  uint16_t bg_x = ((*SCX / 8) + fx) % 32;
  uint16_t bg_y = (*SCY + *LY) & 0xFF;
  uint16_t tile_y = (bg_y / 8) % 32;

  uint16_t background_area = ((*LCDC & 0x08) == 0x08) ? 0x9C00 : 0x9800;
  uint16_t tile_data_area = ((*LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

  uint16_t map_address = background_area + (32 * (tile_y)) + (bg_x);

  uint8_t bg_pixel_index = bus->Vram.read(map_address - 0x8000);

  uint16_t tile_index;
  if (tile_data_area == 0x8000) {
    tile_index = tile_data_area + (uint16_t)(16 * bg_pixel_index);
  } else {
    int8_t signed_index = (int8_t)bg_pixel_index;
    tile_index = 0x9000 + signed_index * 16;
  }

  if (CGB) {
    current_bg_tile.attributes = bus->Vram.read(map_address - 0x8000 + 0x2000);
  }
  uint8_t bank = (current_bg_tile.attributes >> 3) & 0x01;
  int row = (current_bg_tile.attributes >> 6) & 0x01 ? 7 - ((bg_y) % 8)
                                                     : ((bg_y) % 8);
  current_bg_tile.tile_id = tile_index + (2 * row) + (bank * 0x2000);

  current_bg_tile.low_byte = bus->Vram.read((current_bg_tile.tile_id) - 0x8000);

  current_bg_tile.high_byte =
      bus->Vram.read((current_bg_tile.tile_id + 0x0001) - 0x8000);

  for (int i = 0; i < 8; i++) {
    uint8_t x_flip = (current_bg_tile.attributes >> 5) & 0x01;

    uint8_t low_color_bit;
    uint8_t high_color_bit;
    if (x_flip) {
      low_color_bit = (current_bg_tile.low_byte >> (i)) & 0x01;
      high_color_bit = ((current_bg_tile.high_byte >> (i)) & 0x01) << 1;
    } else {
      low_color_bit = (current_bg_tile.low_byte >> (7 - i)) & 0x01;
      high_color_bit = ((current_bg_tile.high_byte >> (7 - i)) & 0x01) << 1;
    }
    uint8_t color = low_color_bit | high_color_bit;

    bg_fifo.push({color, (uint8_t)(current_bg_tile.attributes & 0x07), 0});
  }
}

void Ppu::pixelTransfer() {
  static int last_object = 0;
  if (this->state != Pixeltransfer) {
    this->state = Pixeltransfer;

    *STAT = (*STAT & 0xFC) | 0x03;
    act_obj_index = 0;

    fifo_state = FirstBG;
    fetcher_state = FetchTileId;
    while (!bg_fifo.empty()) {
      bg_fifo.pop();
    }
    lx = -8;
    fx = 0;
    remaining_cycles = 0;
    act_bg_mode = BG;
  }
  obj = nullptr;
  bg_attributes = 0x00;
  pixel_to_draw = 0;
  uint8_t obj_act_pixel = 0;
  int act_obj_index = 0;
  /*if ((*LCDC & 0x02) == 0x02) {
    for (int i = 0; i < obj_index; i++) {
      if (objects[i]->x > 0 && objects[i]->x < 176) {
        if (lx >= objects[i]->x - 8 && lx < objects[i]->x) {
          uint8_t obj_pixel = this->getObjPixel(objects[i]);
          if (obj_pixel != 0) {
            if (obj != nullptr) {
              if ((obj->flags & 0x80) < (objects[i]->flags & 0x80)) {
                obj = objects[i];
                obj_act_pixel = obj_pixel;
                act_obj_index = i;
              }
            } else {
              obj = objects[i];
              obj_act_pixel = obj_pixel;
              act_obj_index = i;
            }
          }
        }
      }
    }
  }*/

  /*if (obj != nullptr && ((obj->flags & 0x80) != 0x80)) {
    this->getObjPixel(obj);
    if (pixel_to_draw != 0)
      return;
  }*/

  if (fifo_state == FirstBG) {
    if (remaining_cycles > 0) {
      remaining_cycles--;
      if (remaining_cycles == 0) {
        fifo_state = BgRender;
        remaining_cycles = 8;
        fx = 0;
        lx -= (*SCX % 8);
      }
      return;
    }
    getBgTile();

    remaining_cycles = 5;

    return;
  }

  if (fifo_state == FirstW) {
    if (remaining_cycles < 5) {
      remaining_cycles--;
      if (remaining_cycles == 0) {
        fifo_state = BgRender;
        remaining_cycles = 8;
        fx = 1;
      }
      return;
    }
    fx = 0;
    getWinTile();
    remaining_cycles--;
    return;
  }

  if (fifo_state == BgRender) {
    if ((*LCDC & 0x20) != 0 && *WY <= *LY && (lx == (int)(*WX - 7)) &&
        act_bg_mode == BG) {
      while (!bg_fifo.empty()) {
        bg_fifo.pop();
      }
      fifo_state = FirstW;
      act_bg_mode = WINDOW;
      remaining_cycles = 5;
      return;
    }
    if (remaining_cycles == 8) {
      if (act_bg_mode == BG) {
        getBgTile();
      } else {
        getWinTile();
      }
      fx++;
    }
    // push
    if (lx >= 0 && !bg_fifo.empty()) {
      BG_pixel_type bg_pixel = bg_fifo.front();
      bg_fifo.pop();
      if (CGB) {
        uint8_t palette = bg_pixel.palette;
        uint8_t color = bg_pixel.color;
        uint8_t bg_priority = bg_pixel.bg_priority;

        uint8_t bg_addr = (palette * 8) + (2 * color);
        *BGPI = (*BGPI & ~(0x3F)) | bg_addr;
        uint8_t low_color = bus->read_bg_cram();
        *BGPI = (*BGPI & ~(0x3F)) | (bg_addr + 1);
        uint8_t high_color = bus->read_bg_cram();
        uint16_t bg_color = (high_color << 8) | low_color;
        this->vga->push_pixel_color(bg_color, lx, *LY);
      } else {
        if (bg_pixel.color != 0) {
          uint8_t bg_color = (*BGP >> (bg_pixel.color * 2)) & 0x03;
          this->vga->push_pixel(bg_color, lx, *LY);
        }
      }
    } else {
      bg_fifo.pop();
    }
    lx++;

    remaining_cycles--;
    if (remaining_cycles == 0) {
      remaining_cycles = 8;
    }
    return;
  }
  return;

  uint8_t bg_pixel = 0;

  if (*LCDC & 0x01 || (CGB && (obj == nullptr || obj_act_pixel == 0))) {
    // window enable in range?
    if ((*LCDC & 0x20) && lx >= (int)(*WX - 7) && *LY >= *WY) {
      bg_pixel = getWinPixel();
    } else {
      bg_pixel = getBgPixel();
    }
  }

  if (CGB) {
    if ((bg_pixel != 0 || obj == nullptr) &&
        ((bg_attributes & 0x80) == 0x80 || obj == nullptr ||
         (obj->flags & 0x80) == 0x80)) {

      uint8_t palette_index = (bg_attributes & 0x07);
      uint8_t color_address = ((palette_index * 8) + (2 * bg_pixel)) & 0x3F;

      *BGPI = (*BGPI & ~(0x3F)) | color_address;
      uint8_t low_color = bus->read_bg_cram();
      *BGPI = (*BGPI & ~(0x3F)) | (color_address + 1);
      uint8_t high_color = bus->read_bg_cram();
      uint16_t bg_color = (high_color << 8) | low_color;
      /*std::cout << std::hex << (int)(2 * bg_pixel) << " " << std::hex
                << (int)color_address << std::endl;*/
      this->vga->push_pixel_color(bg_color, lx, *LY);
      lx++;
      return;
    } else if (obj != nullptr) {

      uint8_t palette_index = obj->flags & 0x07;
      uint8_t color_address =
          ((palette_index * 8) + (2 * obj_act_pixel)) & 0x3F;
      *OBPI = (*OBPI & ~(0x3F)) | color_address;
      uint8_t low_color = bus->read_ob_cram();
      *OBPI = (*OBPI & ~(0x3F)) | (color_address + 1);
      uint8_t high_color = bus->read_ob_cram();
      uint16_t obj_color = (high_color << 8) | low_color;

      this->vga->push_pixel_color(obj_color, lx, *LY);
      lx++;

      return;
    }
    return;
  }

  if (obj_act_pixel == 0 || ((obj->flags & 0x80) == 0x80) && bg_pixel != 0) {
    uint8_t bg_color = (*BGP >> (bg_pixel * 2)) & 0x03;
    this->vga->push_pixel(bg_color, lx, *LY);
  } else {
    uint8_t final_color;
    if ((obj->flags & 0x10) == 0x10) {
      final_color = (*OBP1 >> (obj_act_pixel * 2)) & 0x03;
    } else {
      final_color = (*OBP0 >> (obj_act_pixel * 2)) & 0x03;
    }
    this->vga->push_pixel(final_color, lx, *LY);
  }
}

uint8_t Ppu::getObjPixel(object_type *obj) {
  int obj_x = (lx - (obj->x - 8)) & 0x07;
  if (obj_x < 0)
    obj_x += 8;
  int obj_y = (int)(*LY) - ((int)obj->y - 16);
  uint16_t obj_tile = obj->tile;
  uint8_t obj_flags = obj->flags;

  int sprite_height = (*LCDC & 0x04) ? 16 : 8;

  if (obj_y < 0 || obj_y >= sprite_height) {
    pixel_to_draw = 0;
    return 0;
  }

  uint8_t x_flip = (obj_flags & 0x20) ? 1 : 0;
  uint8_t y_flip = (obj_flags & 0x40) ? 1 : 0;

  int row = y_flip ? (sprite_height - 1 - obj_y) : obj_y;

  if (sprite_height == 16 && row >= 8) {
    row -= 8;
    obj_tile = (obj_tile & 0xFE) | 1;
  }

  uint16_t tile_data_address = 0x8000 + (uint16_t)(16 * obj_tile);

  uint8_t bank = CGB ? (obj_flags >> 3) & 0x01 : 0x00;

  uint8_t obj_pixel_low =
      bus->Vram.read(tile_data_address + (2 * row) - 0x8000 + (bank * 0x2000));
  uint8_t obj_pixel_high = bus->Vram.read(tile_data_address + (2 * row) +
                                          0x0001 - 0x8000 + (bank * 0x2000));

  int pixel_num = x_flip ? obj_x : 7 - obj_x;

  uint8_t obj_pixel = ((obj_pixel_high >> (pixel_num)) & 0x01) << 1 |
                      ((obj_pixel_low >> (pixel_num)) & 0x01);

  pixel_to_draw = obj_pixel;
  return obj_pixel;
}

uint8_t Ppu::getWinPixel() {
  int bg_x = (((lx + 7) - *WX) & 255) / 8;
  int bg_y = ((*LY - *WY) & 255) / 8;

  uint16_t window_area = ((*LCDC & 0x40) == 0x40) ? 0x9C00 : 0x9800;
  uint16_t tile_data_area = ((*LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

  uint16_t map_address = window_area + (32 * (bg_y % 32)) + (bg_x % 32);

  uint8_t bg_pixel_index = bus->Vram.read(map_address - 0x8000);

  uint16_t tile_data_address;
  if (tile_data_area == 0x8000) {
    tile_data_address = tile_data_area + (uint16_t)(16 * bg_pixel_index);
  } else {
    int8_t signed_index = (int8_t)bg_pixel_index;
    tile_data_address = 0x9000 + signed_index * 16;
  }

  uint8_t bg_pixel_low;
  uint8_t bg_pixel_high;
  uint8_t bank = 0x00;
  uint8_t x_flip_value = 7;

  if (CGB) {

    bg_attributes = bus->Vram.read(map_address - 0x8000 + 0x2000);
    bank = (bg_attributes >> 3) & 0x01;

    // is y flip?
    int row =
        (bg_attributes & 0x40) ? 7 - (((*LY - *WY)) % 8) : ((*LY - *WY) % 8);

    bg_pixel_low = bus->Vram.read((tile_data_address + 2 * row) - 0x8000 +
                                  (bank * 0x2000));
    bg_pixel_high = bus->Vram.read((tile_data_address + 2 * row + 0x0001) -
                                   0x8000 + (bank * 0x2000));

    x_flip_value = (bg_attributes & 0x20) ? 0 : 7;
  } else {
    bg_pixel_low = bus->read(tile_data_address + 2 * ((*LY - *WY) % 8));
    bg_pixel_high =
        bus->read(tile_data_address + 2 * ((*LY - *WY) % 8) + 0x0001);

    x_flip_value = 7;
  }

  int col = (lx - (*WX - 7)) % 8;
  int bit_pos = x_flip_value == 7 ? 7 - col : col;

  uint8_t bg_pixel = ((bg_pixel_high >> (bit_pos)) & 0x01) << 1 |
                     ((bg_pixel_low >> (bit_pos)) & 0x01);

  return bg_pixel;
  /*
this->vga->push_pixel(bg_pixel, lx, *LY);*/
}

uint8_t Ppu::getBgPixel() {
  int bg_x = ((*SCX + lx) & 255) / 8;
  int bg_y = ((*SCY + *LY) & 255) / 8;

  uint16_t background_area = ((*LCDC & 0x08) == 0x08) ? 0x9C00 : 0x9800;
  uint16_t tile_data_area = ((*LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

  uint16_t map_address = background_area + (32 * (bg_y % 32)) + (bg_x % 32);

  uint8_t bg_pixel_index = bus->Vram.read(map_address - 0x8000);

  uint16_t tile_data_address;
  if (tile_data_area == 0x8000) {
    tile_data_address = tile_data_area + (uint16_t)(16 * bg_pixel_index);
  } else {
    int8_t signed_index = (int8_t)bg_pixel_index;
    tile_data_address = 0x9000 + signed_index * 16;
  }
  static int count = 0;
  count++;

  uint8_t bg_pixel_low;
  uint8_t bg_pixel_high;
  uint8_t bank = 0x00;
  uint8_t x_flip_value = 7;
  if (CGB) {

    bg_attributes = bus->Vram.read(map_address - 0x8000 + 0x2000);
    bank = (bg_attributes >> 3) & 0x01;

    // is y flip?
    int row =
        (bg_attributes & 0x40) ? 7 - ((*SCY + *LY) % 8) : ((*SCY + *LY) % 8);

    bg_pixel_low = bus->Vram.read((tile_data_address + 2 * row) - 0x8000 +
                                  (bank * 0x2000));
    bg_pixel_high = bus->Vram.read((tile_data_address + 2 * row + 0x0001) -
                                   0x8000 + (bank * 0x2000));

    x_flip_value = (bg_attributes & 0x20) ? 0 : 7;

  } else {
    bg_pixel_low =
        bus->Vram.read((tile_data_address + 2 * ((*SCY + *LY) % 8)) - 0x8000);
    bg_pixel_high = bus->Vram.read(
        (tile_data_address + 2 * ((*SCY + *LY) % 8) + 0x0001) - 0x8000);
  }

  int col = x_flip_value == 0 ? (*SCX + lx) % 8 : 7 - ((*SCX + lx) % 8);

  uint8_t bg_pixel =
      (bg_pixel_high >> col & 0x01) << 1 | (bg_pixel_low >> col & 0x01);

  return bg_pixel;

  // this->vga->push_pixel(bg_pixel, lx, *LY);
}

void Ppu::hBlank() {
  if (this->state != HBlank) {
    this->state = HBlank;

    *STAT = (*STAT & 0xFC) | 0x00;
    this->obj_index = 0;

    //*STAT = (*STAT & 0xA4) | 0x08;

    // request interrupt
    //*IF |= 0x02;
    if ((*STAT & 0x08) == 0x08) {
      // request interrupt

      *IF |= 0x02;
    }
  }
}

void Ppu::vBlank() {
  if (this->state != VBlank) {
    this->state = VBlank;

    *STAT = (*STAT & 0xFC) | 0x01;

    // *STAT = (*STAT & 0xA4) | 0x10;

    // request interrupt
    *IF |= 0x01;
    if ((*STAT & 0x10) == 0x10) {

      *IF |= 0x02;
    }
  }
}

void Ppu::connectBus(Bus *bus) {
  this->bus = bus;
  this->CGB = bus->CGB;

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
  this->BGPI = bus->io.get_address(0x68);
  this->OBPI = bus->io.get_address(0x6A);

  this->WY = bus->io.get_address(0x4A);
  this->WX = bus->io.get_address(0x4B);

  this->IF = bus->get_address(0xFF0F);
  this->IE = bus->get_address(0xFFFF);
}

// restart the ppu without cgb mode
void Ppu::restartToDMG() {
  this->CGB = false;
  *LCDC = 0x91;
  *STAT = 0x85;
  *SCY = 0x00;
  *SCX = 0x00;
  *LY = 0;
  *LYC = 0;
  *DMA = 0x00;

  *BGP = 0xFC;
  *OBP0 = 0xFF;
  *OBP1 = 0xFF;
  *BGPI = 0x00;
  *OBPI = 0x00;

  *WY = 0x00;
  *WX = 0x00;

  *IF = 0x00;
  *IE = 0x00;

  this->cycle_counter = 0;
  this->act_cycles = 0;
  this->lx = 0;
  this->obj_index = 0;
  this->act_obj_index = 0;
}

void Ppu::connectVga(Vga *vga) { this->vga = vga; }

void Ppu::handle_enable_disable() {}

void Ppu::step(uint8_t cycles) {
  static int set_off = 0;
  if (((*LCDC & 0x80) == 0x00) && set_off == 1) {
    set_off = 0;
    if (ppu_was_on) {
      *LY = 0;
      *STAT &= ~(0x03);
      act_cycles = 0;
      cycle_counter = 0;
      lx = 0;
      ppu_was_on = false;
    }
  } else {
    if (!ppu_was_on) {
      set_off = 0;
      *STAT |= 0x01;
      ppu_was_on = true;
    }
    if (((*LCDC & 0x80) == 0x00)) {
      set_off = 1;
    }
    act_cycles += cycles;
    while (act_cycles > 0) {
      if (*LY < 144) {
        if (cycle_counter < 80) {
          oamSearch();
          act_cycles -= 2;
          cycle_counter += 2;
        } else {
          if (*LY < 144 && lx < 160) {
            pixelTransfer();
            act_cycles -= 1;
            cycle_counter += 1;

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
                *STAT |= 0x04;
                if ((*STAT & 0x40) == 0x40) {
                  *IF |= 0x02;
                }
              } else {
                *STAT &= ~0x04;
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
              bus->frame_completed();
            }
            if (*LY == *LYC) {
              *STAT |= 0x04;
              if ((*STAT & 0x40) == 0x40) {
                *IF |= 0x02;
              }
            } else {
              *STAT &= ~0x04;
            }
          }
        }
      }
    }
  }
}