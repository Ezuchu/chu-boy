#include "bus.h"
#include "memory.h"
#include <cstdint>

Bus::Bus() : ram(0x4000), Vram(0x4000) { this->cpu.connectBus(this); }

void Bus::write(uint8_t data, uint16_t address) {}

uint8_t Bus::read(uint16_t address) {
  if (address >= 0x0000 && address <= 0x7FFF) {
    return this->rom->read(address);
  }
  if (address >= 0xC000 && address <= 0xDFFF) {
    return this->ram.read(address - 0xC000);
  }
  if (address >= 0x8000 && address <= 0x9FFF) {
    return this->Vram.read(address - 0x8000);
  }
  return 0x00;
}
