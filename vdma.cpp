#include "vdma.h"
#include "bus.h"
#include "cpu.h"
#include "ppu.h"

VDMA::VDMA(Bus *bus) {
  this->bus = bus;
  this->source_address = 0;
  this->dest_address = 0;
  this->act_address = 0;
  this->cycles = 0;
  this->length = 0;
  this->state = 0;
  this->bytes_written = 0;
  this->hdma5 = bus->get_address(0xFF55);
  this->ppu_prev_state = this->bus->ppu.state;
}

void VDMA::vdma_start(uint8_t mode, uint8_t rlength) {
  if (this->state != 0) {

    if (mode == 0) {
      this->state = 0;
      *hdma5 |= 0x80;
      return;
    }
    this->length = 16 * (uint16_t)(rlength + 1);

    return;
  }

  this->ppu_prev_state = this->bus->ppu.state;
  this->length = 16 * (uint16_t)(rlength + 1);

  if (mode == 0) { // GDMA
    this->state = 1;
    *this->hdma5 = 0xFF;
  } else { // HDMA
    *this->hdma5 &= 0x7F;
    if (bus->ppu.state == HBlank) {
      this->state = 2;
    } else {
      this->state = 3;
    }
  }
}

int VDMA::vdma_step() {

  if (this->state == 0) {
    ppu_prev_state = this->bus->ppu.state;
    return 0;
  }

  if (this->state == 3) {
    if (this->bus->ppu.state == HBlank && ppu_prev_state != HBlank) {
      this->state = 2;
    } else {
      ppu_prev_state = this->bus->ppu.state;
      return 0;
    }
  }
  ppu_prev_state = this->bus->ppu.state;

  int length_count = this->length;
  if (this->state == 1) {
    int overflow = 0;
    while (length != 0 && overflow == 0) {
      uint8_t data = this->read();
      this->write(data);
      this->source_address++;
      this->dest_address++;
      this->length--;
      if (dest_address == 0x0000) {
        overflow = 1;
      }
    }
    this->state = 0;
    *this->hdma5 = 0xFF;
    return length_count;
  }

  if (this->state == 2) {
    int bytes_copied = 0;
    int overflow = 0;
    while (bytes_copied < 16 && length > 0 && overflow == 0) {
      uint8_t data = this->read();
      this->write(data);
      this->source_address++;
      this->dest_address++;
      this->length--;
      bytes_copied++;
      if (dest_address == 0x0000) {
        overflow = 1;
      }
    }
    if (this->length <= 0 || dest_address == 0x0000) {
      this->state = 0;
    } else {
      this->state = 3;
    }

    return bytes_copied;
  }

  return 0;
}

uint8_t VDMA::read_vdma() {
  uint8_t active_bit = this->state == 0 ? 1 : 0;
  uint8_t length_out =
      this->length > 0 ? ((uint8_t)(this->length / 16)) - 1 : 0x7F;
  /*std::cout << (int)active_bit << " " << length << " " << (int)length_out
            << std::endl;*/

  return (active_bit << 7) | (length_out & 0x7F);
}

void VDMA::write_source() {

  this->source_address =
      ((bus->read(0xFF51) << 8) | (bus->read(0xFF52))) & 0xFFF0;

  this->act_address = source_address;
}

void VDMA::write_hdm1() {

  this->source_address =
      (((uint16_t)(bus->read(0xFF51)) << 8) | (source_address & 0x00FF));

  this->act_address = source_address;
}

void VDMA::write_hdm2() {

  this->source_address =
      ((source_address & 0xFF00) | (bus->read(0xFF52) & 0x00F0));

  this->act_address = source_address;
}

void VDMA::write_hdm3() {

  this->dest_address =
      ((((uint16_t)bus->read(0xFF53)) << 8) | (dest_address & 0x00FF));
  this->act_VRAM = dest_address;
}

void VDMA::write_hdm4() {

  this->dest_address = ((dest_address & 0xFF00) | (bus->read(0xFF54) & 0x00F0));
  this->act_VRAM = dest_address;
}

void VDMA::write_dest() {

  this->dest_address =
      ((bus->read(0xFF53) << 8) | (bus->read(0xFF54))) & 0xFFF0;
  this->act_VRAM = dest_address;
}

uint8_t VDMA::read() {
  if ((source_address >= 0x8000 && source_address <= 0x9FFF) || // VRAM
      (source_address >= 0xFE00 && source_address <= 0xFFFF)) { // OAM/IO/HRAM
    return 0xFF;
  }
  return this->bus->read(this->source_address);
}
void VDMA::write(uint8_t data) {
  /*std::cout << "write VDMA " << (int)(act_VRAM & 0x1FFF) << " " << (int)data
            << std::endl;
  std::cout << "address " << 0x8000 + (int)(act_VRAM & 0x1FFF) << " "
            << (int)data << std::endl;*/
  this->bus->write(data, 0x8000 + (dest_address & 0x1FFF));
}