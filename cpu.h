#pragma once

#include "instruction.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#define reg16 uint16_t
#define reg8 uint8_t

#include <cstdio>

class Bus;

class Cpu {
private:
  static FILE *log_file;
  union Register {
    reg16 reg;

    struct {
      uint8_t lower;
      uint8_t higher;
    } Reg8;
  };
  enum flags { z = (0x80), n = (0x40), h = (0x20), cy = (0x10) };
  enum add_type { to_reg, to_memory };

  Register af = {0x01B0}, bc = {0x0013}, de = {0x00D8}, hl = {0x014D};
  reg16 sp = 0xFFFE;
  reg16 pc = 0x0100;

  uint8_t cycles;
  uint8_t remaining_cycle;
  uint8_t opcode;

  uint16_t operand1;
  uint16_t operand2;

  add_type address_type;
  uint16_t address;

  bool bit16 = false;

  bool IME = false;
  bool IME_pending = false;

  bool halt_bug = false;

  uint8_t *IE = nullptr;
  uint8_t *IF = nullptr;
  uint8_t *KEY1 = nullptr;
  Bus *bus = nullptr;

  std::array<instruction, 256> op_table;
  instruction *act_instruction = nullptr;

  void write(uint8_t data, uint16_t address);
  uint8_t read(uint16_t address);

  void fetch_instruction();
  void execute_mode();
  void execute_instruction();

  void fetch();
  void decode();

  uint16_t get_reg(reg_code type);
  void set_reg(reg_code type, uint16_t data);

  uint8_t get_flag(flags flag);
  void change_flag(flags flag);
  void clear_flag(flags flag);
  void set_flag(flags flag);

  void exec_cycle(uint8_t num_cycles);
  void handle_interrupt();
  void push_to_interrupt(uint16_t address);

  bool eval_cond();

  // Address modes
  void NONE();
  void AM_R();
  void AM_I16();
  void AM_R_R();
  void AM_R_D8();
  void AM_R_I16();
  void AM_I16_R();
  void AM_I16_D8();
  void AM_R_a16();
  void AM_a16_R();
  void AM_FFD8_R();
  void AM_R_FFD8();
  void AM_FFI8_R();
  void AM_R_FFI8();
  void AM_RR();
  void AM_RR_NN();
  void AM_a16_RR();
  void AM_RR_RR();
  void AM_SP_S8();
  void AM_HL_SP_S8();

  // Instructions
  void NOP();
  void STOP();
  void HALT();
  void DI();
  void EI();
  void CB();

  void LD();
  void POP();
  void PUSH();

  void INC();
  void DEC();

  void RLA();
  void RLCA();
  void RRA();
  void RRCA();

  void RLC();
  void RRC();
  void RL();
  void RR();
  void SLA();
  void SRA();
  void SWAP();
  void SRL();
  void BIT();
  void RES();
  void SET();

  void ADD();
  void SUB();
  void AND();
  void OR();
  void ADC();
  void SBC();
  void XOR();
  void CP();

  void JR();
  void JP();
  void RET();
  void CALL();
  void RST();
  void RETI();

  void DAA();
  void CPL();
  void SCF();
  void CCF();

public:
  bool CGB = false;
  bool speed_mode = false;
  bool stop_flag = false;
  bool is_halted = false;
  Cpu();

  ~Cpu();

  void connectBus(Bus *bus);
  void restartToDMG();
  void step();
  void print_state();
};
