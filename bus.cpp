#include "bus.h"
#include "memory.h"
#include <cstdint>

Bus::Bus() : ram(0x4000), Vram(0x4000), Oam(0x0100), io(0x0080), hram(0x0080) {
  this->cpu.connectBus(this);
  this->ppu.connectBus(this);
}

void Bus::write(uint8_t data, uint16_t address) {
  if (address >= 0x0000 && address <= 0x7FFF) {
    return;
  }
  if (address >= 0xC000 && address <= 0xDFFF) {
    this->ram.write(data, address - 0xC000);
  }
  if (address >= 0x8000 && address <= 0x9FFF) {
    this->Vram.write(data, address - 0x8000);
  }
  if (address >= 0xFE00 && address <= 0xFE9F) {
    this->Oam.write(data, address - 0xFE00);
  }
  if (address >= 0xFF00 && address <= 0xFF7F) {
    this->io.write(data, address - 0xFF00);
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    this->hram.write(data, address - 0xFF80);
  }
}

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
  if (address >= 0xFE00 && address <= 0xFE9F) {
    return this->Oam.read(address - 0xFE00);
  }
  if (address >= 0xFF00 && address <= 0xFF7F) {
    return this->io.read(address - 0xFF00);
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    return this->hram.read(address - 0xFF80);
  }

  return 0x00;
}

void Bus::clock() { this->cpu.step(); }
