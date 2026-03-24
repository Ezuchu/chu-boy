#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "no_mbc.h"
#include <string>

int main(int argc, char **argv) {
  Cartridge *cart = new Cartridge(argv[1]);

  No_mbc_controller *rom_controller = new No_mbc_controller();
  rom_controller->load_cartridge(cart);

  Bus *bus = new Bus();
  bus->cpu = Cpu();
  bus->cpu.connectBus(bus);

  delete cart;
}