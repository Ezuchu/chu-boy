#include "dma.h"
#include "bus.h"
#include <cstdint>

DMA::DMA(Bus *bus) {
  this->bus = bus;
  this->start_address = 0;
  this->end_address = 0;
  this->act_address = 0;
  this->cycles = 0;
  this->state = false;
}

uint8_t DMA::read() {
  if (this->state) {
    return this->bus->read(this->act_address);
  }
  return 0;
}

void DMA::write(uint8_t data) {
  if (this->state) {
    this->bus->Oam.write(data, (this->act_OAM++) - 0xFE00);
  }
}

void DMA::dma_start(uint8_t start) {
  this->state = true;
  this->act_address = (start << 8) | 0x00;
  this->end_address = this->act_address + 0x9F;
  this->act_OAM = 0xFE00;
}

void DMA::dma_step() {
  if (this->state) {
    this->cycles++;
    uint8_t data = this->read();
    this->write(data);
    this->act_address++;
    if (this->cycles == 160 || act_OAM > 0xFE9F) {
      this->state = false;
      this->cycles = 0;
      this->act_OAM = 0xFE00;
    }
  }
}
