#include "bus.h"
#include "memory.h"
#include <cstdint>

Bus::Bus(bool is_cgb)
    : ram(is_cgb ? 0x8000 : 0x2000), Vram(is_cgb ? 0x4000 : 0x2000),
      Oam(0x0100), io(0x0080), hram(0x0080), Cram(is_cgb ? 0x0080 : 0x0001),
      dma(this), apu() {

  this->CGB = is_cgb;
  this->write(is_cgb ? 0x00 : 0x04, 0xFF4C);

  this->write(0xff, 0xFF00);
  this->tima = this->io.get_address(0x05);
  this->tma = this->io.get_address(0x06);
  this->tac = this->io.get_address(0x07);
  this->div = this->io.get_address(0x04);
  this->IF = this->io.get_address(0x0F);

  this->BGPI = this->io.get_address(0x68);
  this->BGPD = this->io.get_address(0x69);
  this->OBPI = this->io.get_address(0x6A);
  this->OBPD = this->io.get_address(0x6B);

  *div = 0x18;
  *tac = 0xF8;
  *tma = 0x00;
  *tima = 0x00;
  *IF = 0x00;

  this->timer_counter = 0;
  this->div_counter = 0;

  this->apu.connectBus(this);
}

Bus::~Bus() {
  /*delete this->ram;
  delete this->Vram;
  delete this->Oam;
  delete this->io;
  delete this->hram;*/
  std::cout << "delete bus\n";
}

void Bus::write(uint8_t data, uint16_t address) {
  if (address >= 0x0000 && address <= 0x7FFF) {
    this->rom->write(address, data);
  }
  if (address >= 0xC000 && address <= 0xDFFF) {
    if (CGB && address > 0xCFFF) {
      uint8_t RBANK = 0x00;
      RBANK = read(0xFF70) & 0x07;
      if (RBANK == 0)
        RBANK = 0x01;
      this->ram.write(data, address - 0xC000 + (RBANK * 0x1000));
      return;
    }
    this->ram.write(data, address - 0xC000);
  }
  if (address >= 0x8000 && address <= 0x9FFF) {
    if ((this->read(0xFF40) & 0x80) == 0x80 &&
        (this->read(0xFF41) & 0x03) == 0x03)
      return;
    uint8_t VBANK = 0x00;
    // If cgb check video bank
    if (CGB) {
      VBANK = read(0xFF4F) & 0x01;
    }
    this->Vram.write(data, address - 0x8000 + (VBANK * 0x2000));
  }
  if (address >= 0xA000 && address <= 0xBFFF) {
    this->rom->write(address, data);
  }
  if (address >= 0xFE00 && address <= 0xFE9F) {
    if (((this->read(0xFF40) & 0x80) == 0x80 &&
         (this->read(0xFF41) & 0x03) > 0x01) ||
        this->dma.state)
      return;
    this->Oam.write(data, address - 0xFE00);
  }
  if (address >= 0xFF00 && address <= 0xFF7F) {
    if (address == 0xFF00) {
      uint8_t *P1 = this->get_address(0xFF00);
      data = data & 0x30;
      *P1 = (data & 0x30) | (*P1 & 0x0F);
      return;
    }
    if (address == 0xFF04) {
      this->div_counter = 0;
      *div = 0x00;
      return;
    }
    if (address == 0xFF0F) {
      *IF = data | 0xE0;
      return;
    }
    this->io.write(data, address - 0xFF00);
    if (CGB) {
      if (address == 0xFF69) {
        this->write_bg_cram(data);
        return;
      }
      if (address == 0xFF6B) {
        this->write_ob_cram(data);
        return;
      }
    }
    if (address == 0xFF46) {
      this->dma.dma_start(data);
    }
    switch (address) {
    case 0xFF14:
      this->apu.update_channel_reg1();
      break;
    case 0xFF19:
      this->apu.update_channel_reg2();
      break;
    case 0xFF1E:
      this->apu.update_channel_reg3();
      break;
    case 0xFF23:
      this->apu.update_channel_reg4();
      break;
    case 0xFF26:
      if (data & 0x80) {
        this->apu.update_channel_reg1();
        this->apu.update_channel_reg2();
        this->apu.update_channel_reg3();
        this->apu.update_channel_reg4();
      }
      break;
    default:
      break;
    }
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    this->hram.write(data, address - 0xFF80);
  }
}

uint8_t Bus::read(uint16_t address, bool is_cpu) {

  if (address <= 0x7FFF) {
    if (this->rom == nullptr) {
      return 0xFF; // Default value if no ROM is loaded
    }
    return this->rom->read(address);
  }
  if (address >= 0xC000 && address <= 0xDFFF) {
    if (CGB && address > 0xCFFF) {
      uint8_t RBANK = 0x00;
      RBANK = read(0xFF70) & 0x07;
      if (RBANK == 0)
        RBANK = 0x01;
      return this->ram.read(address - 0xC000 + (RBANK * 0x1000));
    }
    return this->ram.read(address - 0xC000);
  }
  if (address >= 0x8000 && address <= 0x9FFF) {
    // Vram check access for cpu
    if ((this->read(0xFF40) & 0x80) == 0x80 &&
        (this->read(0xFF41) & 0x03) == 0x03 && is_cpu)
      return 0xFF;
    uint8_t VBANK = 0x00;
    if (CGB) {
      VBANK = read(0xFF4F) & 0x01;
    }
    return this->Vram.read(address - 0x8000 + (VBANK * 0x2000));
  }
  if (address >= 0xA000 && address <= 0xBFFF) {
    return this->rom->read(address);
  }
  if (address >= 0xFE00 && address <= 0xFE9F) {
    if ((this->read(0xFF40) & 0x80) == 0x80 &&
        (this->read(0xFF41) & 0x03) > 0x01 && is_cpu)
      return 0xFF;
    return this->Oam.read(address - 0xFE00);
  }
  if (address >= 0xFF00 && address <= 0xFF7F) {
    /*if (address == 0xFF44) {
      return 0x90;
    }*/

    if (address == 0xFF69 && CGB) {
      return this->read_bg_cram();
    }
    if (address == 0xFF6B && CGB) {
      return this->read_ob_cram();
    }
    return this->io.read(address - 0xFF00);
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    return this->hram.read(address - 0xFF80);
  }

  return 0x00;
}

void Bus::write_bg_cram(uint8_t data) {
  uint8_t address = *BGPI & 0x3F;
  this->Cram.write(data, address);
  if (*BGPI & 0x80) {
    *BGPI = (*BGPI + 0x01) & 0xBF;
  }
}

void Bus::write_ob_cram(uint8_t data) {
  uint8_t address = *OBPI & 0x3F;
  this->Cram.write(data, address + 0x40);
  if (*OBPI & 0x80) {
    *OBPI = (*OBPI + 0x01) & 0xBF;
  }
}

uint8_t Bus::read_bg_cram() {
  uint8_t address = *BGPI & 0x3F;
  if (*BGPI & 0x80) {
    *BGPI = (*BGPI + 0x01) & 0xBF;
  }
  return this->Cram.read(address);
}

uint8_t Bus::read_ob_cram() {
  uint8_t address = *OBPI & 0x3F;
  if (*OBPI & 0x80) {
    *OBPI = (*OBPI + 0x01) & 0xBF;
  }
  return this->Cram.read(address + 0x40);
}

uint8_t *Bus::get_address(uint16_t address) {
  if (address <= 0x7FFF) {
    return nullptr;
  }
  if (address >= 0xC000 && address <= 0xDFFF) {
    if (CGB && address > 0xCFFF) {
      uint8_t RBANK = 0x00;
      RBANK = read(0xFF70) & 0x07;
      if (RBANK == 0)
        RBANK = 0x01;
      return this->ram.get_address(address - 0xC000 + (RBANK * 0x1000));
    }
    return this->ram.get_address(address - 0xC000);
  }
  if (address >= 0x8000 && address <= 0x9FFF) {
    uint8_t VBANK = 0x00;
    if (CGB) {
      VBANK = read(0xFF4F) & 0x01;
    }
    return this->Vram.get_address(address - 0x8000 + (VBANK * 0x2000));
  }
  if (address >= 0xFE00 && address <= 0xFE9F) {
    return this->Oam.get_address(address - 0xFE00);
  }
  if (address >= 0xFF00 && address <= 0xFF7F) {
    return this->io.get_address(address - 0xFF00);
  }
  if (address >= 0xFF80 && address <= 0xFFFF) {
    return this->hram.get_address(address - 0xFF80);
  }

  return nullptr;
}

void Bus::clock() {
  if (this->cpu.stop_flag) {
    this->cpu.step();
    return;
  }
  this->cpu.step();
  this->dma.dma_step();
  this->apu.step(this->cpu.speed_mode ? 2 : 4);
  this->ppu.step(this->cpu.speed_mode ? 2 : 4);

  this->div_counter += 1;
  if (div_counter == 64) {
    *div += 1;
    div_counter = 0;
  }
  timer_clock(1);
}

void Bus::clock(uint8_t cycles) {
  for (int i = 0; i < cycles; i++) {
    this->dma.dma_step();
    this->apu.step(this->cpu.speed_mode ? 2 : 4);
    this->ppu.step(this->cpu.speed_mode ? 2 : 4);

    this->div_counter += 1;
    if (this->div_counter == 64) {
      *div += 1;
      this->div_counter = 0;
    }
    timer_clock(1);
  }
}

void Bus::frame_completed() {
  this->frame_counter++;
  if (frame_counter >= save_frame_interval) {
    this->rom->save_state();
    this->frame_counter = 0;
  }
}

void Bus::timer_clock(uint8_t cycles) {
  static uint16_t cycle_select[4] = {256, 4, 16, 64};
  if ((*tac & 0x04) == 0x04) {
    timer_counter += cycles;
    if (timer_counter >= cycle_select[(*tac & 0x03)]) {
      timer_counter -= cycle_select[(*tac & 0x03)];
      *tima += 1;
      if (*tima == 0x00) {
        *tima = *tma;
        *IF |= 0x04;
      }
    }
  }
}
