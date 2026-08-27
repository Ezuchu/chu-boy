#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "joyPad.h"
#include "mbc.h"
#include "vga.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_scancode.h>
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

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, 160, 144);

  Cartridge *cart = new Cartridge(argv[1]);

  rom_controller *mbc = mbc_factory::create_mbc(cart);

  if (!mbc) {
    std::cout << "Could not create mbc" << std::endl;
    return 1;
  }
  mbc->load_cartridge(cart);

  uint16_t key_config[8] = {SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_U,
                            SDL_SCANCODE_I, SDL_SCANCODE_W, SDL_SCANCODE_S,
                            SDL_SCANCODE_A, SDL_SCANCODE_D};

  bool CGB = (cart->cgb_flag == 0x80) || (cart->cgb_flag == 0xC0);
  Bus *bus = new Bus(CGB);
  bus->rom = mbc;
  bus->cpu = Cpu();
  bus->cpu.connectBus(bus);
  bus->ppu = Ppu();
  bus->ppu.connectBus(bus);

  JoyPad *joypad = new JoyPad(key_config);
  joypad->connectBus(bus);

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
    joypad->get_state(SDL_GetKeyboardState(NULL));
    bus->clock();
  }

  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();

  delete mbc;
  delete bus;
  delete vga;

  delete cart;
  return 0;
}