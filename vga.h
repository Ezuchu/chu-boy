#pragma once

#include <cstdint>

class SDL_Renderer;
class SDL_Texture;

class Vga {
  uint32_t buffer[160 * 144];

public:
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  Vga();
  ~Vga();

  void push_pixel(uint8_t pixel, uint8_t x, uint8_t y);
  void push_pixel_color(uint16_t pixel, uint8_t x, uint8_t y);
  void clear_buffer();
  void render();
};