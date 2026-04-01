#include "vga.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <cstdint>

Vga::Vga() { clear_buffer(); }

Vga::~Vga() {}

void Vga::push_pixel(uint8_t pixel, uint8_t x, uint8_t y) {
  uint32_t colors[] = {0x00e0f8d0, 0x0088c070, 0x00346856, 0x000f380f};
  buffer[(y * 160) + x] = colors[pixel];
}

void Vga::clear_buffer() {
  for (int i = 0; i < 160 * 144; i++) {
    buffer[i] = 0;
  }
}

void Vga::render() {
  SDL_UpdateTexture(texture, nullptr, buffer, 160 * sizeof(uint32_t));

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);

  SDL_Delay(16);
}