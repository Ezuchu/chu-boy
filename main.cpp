#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "no_mbc.h"
#include "vga.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

const int window_width = 160;
const int window_height = 144;

int main(int argc, char **argv) {
  if (argc < 2) {
    SDL_Log("Usage: %s <ROM path>", argv[0]);
    return 1;
  }

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Chu-Boy", window_width, window_height,
                                        SDL_WINDOW_RESIZABLE);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);

  SDL_Texture *texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 160, 144);

  Cartridge *cart = new Cartridge(argv[1]);

  No_mbc_controller *rom_controller = new No_mbc_controller();

  rom_controller->load_cartridge(cart);

  Bus *bus = new Bus();
  bus->cpu = Cpu();
  bus->cpu.connectBus(bus);
  bus->ppu = Ppu();
  bus->ppu.connectBus(bus);
  bus->rom = rom_controller;

  Vga *vga = new Vga();
  bus->vga = vga;
  bus->ppu.connectVga(vga);
  vga->renderer = renderer;
  vga->texture = texture;

  SDL_Event event;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) { // Handle the window close button
        quit = true;
      }
      // Handle other events here (keyboard, mouse, etc.)
    }
    bus->clock();
  }

  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();

  delete cart;
  delete bus;
  delete vga;
  delete rom_controller;
  return 0;
}