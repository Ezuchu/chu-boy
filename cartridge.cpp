#include "cartridge.h"

Cartridge::Cartridge(std::string rom_name) {
  this->rom_data = nullptr;
  std::ifstream rom_file;

  rom_file.open(rom_name, std::ifstream::binary);

  if (!rom_file.is_open()) {
    std::cerr << "Failed to open rom: " << rom_name << std::endl;
    return;
  }

  rom_file.seekg(0x134);
  rom_file.read(this->game_title, 16);
  rom_file.get(reinterpret_cast<char &>(this->cgb_flag));
  rom_file.read(this->license, 2);
  rom_file.seekg((int)rom_file.tellg() + 0x1);

  rom_file.seekg(0x0147);
  rom_file.get(reinterpret_cast<char &>(this->cartridge_type));

  rom_file.seekg(0x0148);
  rom_file.get(reinterpret_cast<char &>(this->rom_size));

  rom_file.seekg(0x0149);
  rom_file.get(reinterpret_cast<char &>(this->ram_size));

  rom_file.seekg(0x0);

  this->rom_data = new uint8_t[0x8000 * (1 << this->rom_size)];

  rom_file.read(reinterpret_cast<char *>(rom_data),
                0x8000 * (1 << this->rom_size));

  rom_file.close();

  this->output_rom_data();
};

void Cartridge::output_rom_data() {
  std::cout << "Game Title: " << this->game_title << std::endl;
  std::cout << "CGB Flag: " << (int)this->cgb_flag << std::endl;
  std::cout << "License: " << this->license << std::endl;
  std::cout << "Cartridge Type: " << (int)this->cartridge_type << std::endl;
  std::cout << "ROM Size: " << (int)this->rom_size << std::endl;
  std::cout << "RAM Size: " << (int)this->ram_size << std::endl;
};

uint8_t *Cartridge::return_rom_data() { return this->rom_data; }