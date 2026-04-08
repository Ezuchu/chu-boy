#include "joyPad.h"
#include "bus.h"
#include <cstdint>

JoyPad::JoyPad(uint16_t *codes) {
  A = codes[0];
  B = codes[1];
  SELECT = codes[2];
  START = codes[3];
  UP = codes[4];
  DOWN = codes[5];
  LEFT = codes[6];
  RIGHT = codes[7];
}

JoyPad::~JoyPad() {}

void JoyPad::connectBus(Bus *bus) {
  this->bus = bus;
  this->P1 = bus->get_address(0xFF00);
}

void JoyPad::get_state(const bool *key_state) {
  bool dPad = false;
  uint8_t state = *P1 | 0x0F;

  if ((*P1 & 0x10) == 0x00) {
    dPad = true;
    if (key_state[UP]) {
      state &= (uint8_t)(~0x04);
    }
    if (key_state[DOWN]) {
      state &= (uint8_t)~0x08;
    }
    if (key_state[LEFT]) {
      state &= (uint8_t)~0x02;
    }
    if (key_state[RIGHT]) {
      state &= (uint8_t)~0x01;
    }
  }

  if (!dPad && ((*P1 & 0x20) == 0x00)) {
    if (key_state[A]) {
      state &= (uint8_t)~0x01;
    }
    if (key_state[START]) {
      state &= (uint8_t)~0x08;
    }
    if (key_state[B]) {
      state &= (uint8_t)~0x02;
    }
    if (key_state[SELECT]) {
      state &= (uint8_t)~0x04;
    }
  }
  *P1 = state;
}