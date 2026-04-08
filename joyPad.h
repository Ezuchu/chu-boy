#pragma once

#include <cstdint>

class Bus;

class JoyPad {
  uint16_t A;
  uint16_t B;
  uint16_t SELECT;
  uint16_t START;
  uint16_t UP;
  uint16_t DOWN;
  uint16_t LEFT;
  uint16_t RIGHT;

public:
  Bus *bus;
  uint8_t *P1;

  void set_button(uint16_t button);
  void unset_button(uint16_t button);
  void get_state(const bool *state);
  void connectBus(Bus *bus);

  JoyPad(uint16_t *codes);
  ~JoyPad();
};