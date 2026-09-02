#include "vdma.h"
#include "bus.h"
#include "cpu.h"

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
}

void VDMA::vdma_start(uint8_t mode, uint8_t rlength) {
  if (this->state != 0) {
    if (mode == 0) {
      this->state = 0;
      *this->hdma5 = 0xFF;
      this->bytes_written = 0;
      return;
    }
    *hdma5 = (*hdma5 & 0x80) | rlength;
    this->length = 16 * (rlength + 1);
  }

  this->source_address =
      ((bus->read(0xFF51) << 8) | (bus->read(0xFF52))) & 0xFFF0;
  this->dest_address =
      ((bus->read(0xFF53) << 8) | (bus->read(0xFF54))) & 0x1FF0;

  this->length = 16 * (rlength + 1);
  this->act_address = this->source_address;
  this->act_VRAM = 0x8000 + dest_address;

  this->state = mode == 0 ? 1 : 2;

  this->bytes_written = 0;
  if (mode == 0) { // GDMA
    this->state = 1;
    *this->hdma5 = 0xFF;
  } else { // HDMA
    this->state = 2;
    *this->hdma5 = 0x80 | (rlength & 0x7F);
  }
}

void VDMA::vdma_step(uint8_t bytes) {
  uint8_t act_bytes = bytes;

  if (this->state == 3) {
    if ((bus->read(0xFF41) & 0x03) != 0) {
      this->state = 2;
    }
    return;
  }

  if (this->state == 1 ||
      (this->state == 2 && (bus->read(0xFF41) & 0x03) == 0 &&
       ((this->bus->read(0xFF40) & 0x80) != 0x00) &&
       !this->bus->cpu.is_halted)) {

    while (act_bytes != 0 && bytes_written < 16) {
      uint8_t data = this->read();
      this->write(data);
      this->act_address++;
      this->act_VRAM++;
      act_bytes--;
      bytes_written++;
      if (bytes_written == 16) {
        if (this->state == 2) {
          *hdma5 -= 0x01;
          this->state = 3;
        } else {
          bytes_written = 0;
        }
      }
      if (act_VRAM > 0x9FFF) {
        this->state = 0;
        *this->hdma5 = 0xFF;
        return;
      }
    }
  }
}

uint8_t VDMA::read() { return this->bus->read(this->act_address); }
void VDMA::write(uint8_t data) { this->bus->write(data, act_VRAM); };