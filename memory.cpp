#include "memory.h"

Memory::Memory(uint16_t size) {
  this->data = new uint8_t[size];
  this->size = size;

  for (uint16_t i = 0; i < size; i++) {
    this->data[i] = 0x00;
  }
}

Memory::~Memory() { delete[] this->data; }

uint8_t Memory::read(uint16_t address) { return this->data[address]; }

void Memory::write(uint8_t data, uint16_t address) {
  this->data[address] = data;
}