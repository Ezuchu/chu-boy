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
  this->source_address =
      ((bus->read(0xFF51) << 8) | (bus->read(0xFF52))) & 0xFFF0;
  this->dest_address =
      ((bus->read(0xFF53) << 8) | (bus->read(0xFF54))) & 0x1FF0;

  this->length = 16 * (rlength + 1);
  this->act_address = this->source_address;
  this->act_VRAM = 0x8000 + dest_address;

  this->state = mode == 0 ? 1 : 2;
  this->bus->cpu.is_halted = state == 1 ? true : this->bus->cpu.is_halted;
  this->bytes_written = 0;
}

void VDMA::vdma_step(uint8_t bytes) {
  uint8_t act_bytes = bytes;
  if (this->state == 1) {
    while (bytes != 0) {
      uint8_t data = this->read();
      this->write(data);
      this->act_address++;
      this->act_VRAM++;
      bytes--;
      bytes_written++;
      if ((bytes_written % 16) == 0) {
        *hdma5 -= 0x01;
      }
      if (bytes_written >= length) {
        this->state = 0;
        *this->hdma5 &= 0x7FFF;
        this->bus->cpu.is_halted = false;
        return;
      }
    }
  }
}

uint8_t VDMA::read() { return this->bus->read(this->act_address); }
void VDMA::write(uint8_t data) { this->bus->write(data, act_VRAM); };