#include "vga.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <cstdint>

Vga::Vga() { clear_buffer(); }

Vga::~Vga() {}

void Vga::push_pixel(uint8_t pixel, uint8_t x, uint8_t y) {
  uint32_t colors[] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};
  buffer[(y * 160) + x] = colors[pixel];
}

void Vga::push_pixel_color(uint16_t pixel, uint8_t x, uint8_t y) {
  // transform color from rgb555 to rgb888
  uint8_t red = pixel & 0x001F;
  uint8_t green = (pixel & 0x03E0) >> 5;
  uint8_t blue = (pixel & 0x7C00) >> 10;

  red = (red << 3) | (red >> 2);
  green = (green << 3) | (green >> 2);
  blue = (blue << 3) | (blue >> 2);

  buffer[(y * 160) + x] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
}

void Vga::clear_buffer() {
  for (int i = 0; i < 160 * 144; i++) {
    buffer[i] = 0xFFFFFFFF;
  }
}

void Vga::render() {
  SDL_UpdateTexture(texture, nullptr, buffer, 160 * sizeof(uint32_t));

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);

  clear_buffer();

  SDL_Delay(16);
}