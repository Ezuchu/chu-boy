#include "bus.h"
#include "memory.h"
#include <cstdint>

Bus::Bus()
    : ram(0x4000), Vram(0x4000), Oam(0x0100), io(0x0080), hram(0x0080),
      dma(this) {
  this->write(0xff, 0xFF00);
  this->tima = this->io.get_address(0x05);
  this->tma = this->io.get_address(0x06);
  this->tac = this->io.get_address(0x07);
  this->div = this->io.get_address(0x04);
  this->IF = this->io.get_address(0x0F);

  *div = 0x18;
  *tac = 0xF8;
  *tma = 0x00;
  *tima = 0x00;
  *IF = 0x00;
}

Bus::~Bus() {}

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
    if (address == 0xFF00) {
      uint8_t *P1 = this->get_address(0xFF00);
      data = data & 0x30;
      *P1 = (data & 0x30) | (*P1 & 0x0F);
      return;
    }
    this->io.write(data, address - 0xFF00);
    if (address == 0xFF46) {
      this->dma.dma_start(data);
    }
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    this->hram.write(data, address - 0xFF80);
  }
}

uint8_t Bus::read(uint16_t address) {

  if (address <= 0x7FFF) {
    if (this->rom == nullptr) {
      return 0xFF; // Default value if no ROM is loaded
    }
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
    /*if (address == 0xFF44) {
      return 0x90;
    }*/
    return this->io.read(address - 0xFF00);
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    return this->hram.read(address - 0xFF80);
  }

  return 0x00;
}

uint8_t *Bus::get_address(uint16_t address) {
  if (address <= 0x7FFF) {
    return nullptr;
  }
  if (address >= 0xC000 && address <= 0xDFFF) {
    return this->ram.get_address(address - 0xC000);
  }
  if (address >= 0x8000 && address <= 0x9FFF) {
    return this->Vram.get_address(address - 0x8000);
  }
  if (address >= 0xFE00 && address <= 0xFE9F) {
    return this->Oam.get_address(address - 0xFE00);
  }
  if (address >= 0xFF00 && address <= 0xFF7F) {
    return this->io.get_address(address - 0xFF00);
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    return this->hram.get_address(address - 0xFF80);
  }

  return nullptr;
}

void Bus::clock() {
  this->cpu.step();
  this->dma.dma_step();
  this->ppu.step(4);

  this->div_counter += 1;
  if (div_counter == 64) {
    *div += 1;
    div_counter = 0;
  }
  timer_clock(1);
}

void Bus::clock(uint8_t cycles) {
  for (int i = 0; i < cycles; i++) {
    this->dma.dma_step();
    this->ppu.step(4);
    this->div_counter += 1;
    if (div_counter == 64) {
      *div += 1;
      div_counter = 0;
    }
    timer_clock(1);
  }
}

void Bus::timer_clock(uint8_t cycles) {
  static uint16_t cycle_select[4] = {256, 4, 16, 64};
  if ((*tac & 0x04) == 0x04) {
    timer_counter += cycles;
    if (timer_counter >= cycle_select[(*tac & 0x03)]) {
      timer_counter -= cycle_select[(*tac & 0x03)];
      *tima += 1;
      if (*tima == 0x00) {
        *tima = *tma;
        *IF |= 0x04;
      }
    }
  }
}
