#include "cpu.h"
#include "bus.h"
#include "instruction.h"
#include <cstdint>

Cpu::Cpu() {
  pc = 0x100;

  op_table[0x00] = {&Cpu::NOP, &Cpu::NONE, None, None, No_cond};
  op_table[0x01] = {&Cpu::LD, &Cpu::AM_RR_NN, regBC, None, No_cond};
  op_table[0x02] = {&Cpu::LD, &Cpu::AM_I16_R, regBC, regA, No_cond};
  op_table[0x03] = {&Cpu::INC, &Cpu::AM_RR, regBC, None, No_cond};
  op_table[0x04] = {&Cpu::INC, &Cpu::AM_R, regB, None, No_cond};
  op_table[0x05] = {&Cpu::DEC, &Cpu::AM_R, regB, None, No_cond};
  op_table[0x06] = {&Cpu::LD, &Cpu::AM_R_D8, regB, None, No_cond};
  op_table[0x07] = {&Cpu::RLCA, &Cpu::NONE, None, None, No_cond};
  op_table[0x08] = {&Cpu::LD, &Cpu::AM_a16_RR, regSP, None, No_cond};
  op_table[0x09] = {&Cpu::ADD, &Cpu::AM_RR_RR, regHL, regBC, No_cond};
  op_table[0x0A] = {&Cpu::LD, &Cpu::AM_R_I16, regA, regBC, No_cond};
  op_table[0x0B] = {&Cpu::DEC, &Cpu::AM_RR, regBC, None, No_cond};
  op_table[0x0C] = {&Cpu::INC, &Cpu::AM_R, regC, None, No_cond};
  op_table[0x0D] = {&Cpu::DEC, &Cpu::AM_R, regC, None, No_cond};
  op_table[0x0E] = {&Cpu::LD, &Cpu::AM_R_D8, regC, None, No_cond};
  op_table[0x0F] = {&Cpu::RRCA, &Cpu::NONE, None, None, No_cond};

  op_table[0x10] = {&Cpu::STOP, &Cpu::AM_R_D8, None, None, No_cond};
  op_table[0x11] = {&Cpu::LD, &Cpu::AM_RR_NN, regDE, None, No_cond};
  op_table[0x12] = {&Cpu::LD, &Cpu::AM_I16_R, regDE, regA, No_cond};
  op_table[0x13] = {&Cpu::INC, &Cpu::AM_RR, regDE, None, No_cond};
  op_table[0x14] = {&Cpu::INC, &Cpu::AM_R, regD, None, No_cond};
  op_table[0x15] = {&Cpu::DEC, &Cpu::AM_R, regD, None, No_cond};
  op_table[0x16] = {&Cpu::LD, &Cpu::AM_R_D8, regD, None, No_cond};
  op_table[0x17] = {&Cpu::RLA, &Cpu::NONE, None, None, No_cond};
  op_table[0x18] = {&Cpu::JR, &Cpu::AM_R_D8, None, None, No_cond};
  op_table[0x19] = {&Cpu::ADD, &Cpu::AM_RR_RR, regHL, regDE, No_cond};
  op_table[0x1A] = {&Cpu::LD, &Cpu::AM_R_I16, regA, regDE, No_cond};
  op_table[0x1B] = {&Cpu::DEC, &Cpu::AM_RR, regDE, None, No_cond};
  op_table[0x1C] = {&Cpu::INC, &Cpu::AM_R, regE, None, No_cond};
  op_table[0x1D] = {&Cpu::DEC, &Cpu::AM_R, regE, None, No_cond};
  op_table[0x1E] = {&Cpu::LD, &Cpu::AM_R_D8, regE, None, No_cond};
  op_table[0x1F] = {&Cpu::RRA, &Cpu::NONE, None, None, No_cond};

  op_table[0x20] = {&Cpu::JR, &Cpu::AM_R_D8, None, None, NZ_cond};
  op_table[0x21] = {&Cpu::LD, &Cpu::AM_RR_NN, regHL, None, No_cond};
  op_table[0x22] = {&Cpu::LD, &Cpu::AM_I16_R, regHLaddI, regA, No_cond};
  op_table[0x23] = {&Cpu::INC, &Cpu::AM_RR, regHL, None, No_cond};
  op_table[0x24] = {&Cpu::INC, &Cpu::AM_R, regH, None, No_cond};
  op_table[0x25] = {&Cpu::DEC, &Cpu::AM_R, regH, None, No_cond};
  op_table[0x26] = {&Cpu::LD, &Cpu::AM_R_D8, regH, None, No_cond};
  op_table[0x27] = {&Cpu::DDA, &Cpu::NONE, None, None, No_cond};
  op_table[0x28] = {&Cpu::JR, &Cpu::AM_R_D8, None, None, Z_cond};
  op_table[0x29] = {&Cpu::ADD, &Cpu::AM_RR_RR, regHL, regHL, No_cond};
  op_table[0x2A] = {&Cpu::LD, &Cpu::AM_R_I16, regA, regHLaddI, No_cond};
  op_table[0x2B] = {&Cpu::DEC, &Cpu::AM_RR, regHL, None, No_cond};
  op_table[0x2C] = {&Cpu::INC, &Cpu::AM_R, regL, None, No_cond};
  op_table[0x2D] = {&Cpu::DEC, &Cpu::AM_R, regL, None, No_cond};
  op_table[0x2E] = {&Cpu::LD, &Cpu::AM_R_D8, regL, None, No_cond};
  op_table[0x2F] = {&Cpu::CPL, &Cpu::NONE, None, None, No_cond};

  op_table[0x30] = {&Cpu::JR, &Cpu::AM_R_D8, None, None, NC_cond};
  op_table[0x31] = {&Cpu::LD, &Cpu::AM_RR_NN, regSP, None, No_cond};
  op_table[0x32] = {&Cpu::LD, &Cpu::AM_I16_R, regHLaddD, regA, No_cond};
  op_table[0x33] = {&Cpu::INC, &Cpu::AM_RR, regSP, None, No_cond};
  op_table[0x34] = {&Cpu::INC, &Cpu::AM_I16, regHL, None, No_cond};
  op_table[0x35] = {&Cpu::DEC, &Cpu::AM_I16, regHL, None, No_cond};
  op_table[0x36] = {&Cpu::LD, &Cpu::AM_I16_D8, regHL, None, No_cond};
  op_table[0x37] = {&Cpu::SCF, &Cpu::NONE, None, None, No_cond};
  op_table[0x38] = {&Cpu::JR, &Cpu::AM_R_D8, None, None, C_cond};
  op_table[0x39] = {&Cpu::ADD, &Cpu::AM_RR_RR, regHL, regSP, No_cond};
  op_table[0x3A] = {&Cpu::LD, &Cpu::AM_R_I16, regA, regHLaddD, No_cond};
  op_table[0x3B] = {&Cpu::DEC, &Cpu::AM_RR, regSP, None, No_cond};
  op_table[0x3C] = {&Cpu::INC, &Cpu::AM_R, regA, None, No_cond};
  op_table[0x3D] = {&Cpu::DEC, &Cpu::AM_R, regA, None, No_cond};
  op_table[0x3E] = {&Cpu::LD, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0x3F] = {&Cpu::CCF, &Cpu::NONE, None, None, No_cond};

  op_table[0x40] = {&Cpu::LD, &Cpu::AM_R_R, regB, regB, No_cond};
  op_table[0x41] = {&Cpu::LD, &Cpu::AM_R_R, regB, regC, No_cond};
  op_table[0x42] = {&Cpu::LD, &Cpu::AM_R_R, regB, regD, No_cond};
  op_table[0x43] = {&Cpu::LD, &Cpu::AM_R_R, regB, regE, No_cond};
  op_table[0x44] = {&Cpu::LD, &Cpu::AM_R_R, regB, regH, No_cond};
  op_table[0x45] = {&Cpu::LD, &Cpu::AM_R_R, regB, regL, No_cond};
  op_table[0x46] = {&Cpu::LD, &Cpu::AM_R_I16, regB, regHL, No_cond};
  op_table[0x47] = {&Cpu::LD, &Cpu::AM_R_R, regB, regA, No_cond};
  op_table[0x48] = {&Cpu::LD, &Cpu::AM_R_R, regC, regB, No_cond};
  op_table[0x49] = {&Cpu::LD, &Cpu::AM_R_R, regC, regC, No_cond};
  op_table[0x4A] = {&Cpu::LD, &Cpu::AM_R_R, regC, regD, No_cond};
  op_table[0x4B] = {&Cpu::LD, &Cpu::AM_R_R, regC, regE, No_cond};
  op_table[0x4C] = {&Cpu::LD, &Cpu::AM_R_R, regC, regH, No_cond};
  op_table[0x4D] = {&Cpu::LD, &Cpu::AM_R_R, regC, regL, No_cond};
  op_table[0x4E] = {&Cpu::LD, &Cpu::AM_R_I16, regC, regHL, No_cond};
  op_table[0x4F] = {&Cpu::LD, &Cpu::AM_R_R, regC, regA, No_cond};

  op_table[0x50] = {&Cpu::LD, &Cpu::AM_R_R, regD, regB, No_cond};
  op_table[0x51] = {&Cpu::LD, &Cpu::AM_R_R, regD, regC, No_cond};
  op_table[0x52] = {&Cpu::LD, &Cpu::AM_R_R, regD, regD, No_cond};
  op_table[0x53] = {&Cpu::LD, &Cpu::AM_R_R, regD, regE, No_cond};
  op_table[0x54] = {&Cpu::LD, &Cpu::AM_R_R, regD, regH, No_cond};
  op_table[0x55] = {&Cpu::LD, &Cpu::AM_R_R, regD, regL, No_cond};
  op_table[0x56] = {&Cpu::LD, &Cpu::AM_R_I16, regD, regHL, No_cond};
  op_table[0x57] = {&Cpu::LD, &Cpu::AM_R_R, regD, regA, No_cond};
  op_table[0x58] = {&Cpu::LD, &Cpu::AM_R_R, regE, regB, No_cond};
  op_table[0x59] = {&Cpu::LD, &Cpu::AM_R_R, regE, regC, No_cond};
  op_table[0x5A] = {&Cpu::LD, &Cpu::AM_R_R, regE, regD, No_cond};
  op_table[0x5B] = {&Cpu::LD, &Cpu::AM_R_R, regE, regE, No_cond};
  op_table[0x5C] = {&Cpu::LD, &Cpu::AM_R_R, regE, regH, No_cond};
  op_table[0x5D] = {&Cpu::LD, &Cpu::AM_R_R, regE, regL, No_cond};
  op_table[0x5E] = {&Cpu::LD, &Cpu::AM_R_I16, regE, regHL, No_cond};
  op_table[0x5F] = {&Cpu::LD, &Cpu::AM_R_R, regE, regA, No_cond};

  op_table[0x60] = {&Cpu::LD, &Cpu::AM_R_R, regH, regB, No_cond};
  op_table[0x61] = {&Cpu::LD, &Cpu::AM_R_R, regH, regC, No_cond};
  op_table[0x62] = {&Cpu::LD, &Cpu::AM_R_R, regH, regD, No_cond};
  op_table[0x63] = {&Cpu::LD, &Cpu::AM_R_R, regH, regE, No_cond};
  op_table[0x64] = {&Cpu::LD, &Cpu::AM_R_R, regH, regH, No_cond};
  op_table[0x65] = {&Cpu::LD, &Cpu::AM_R_R, regH, regL, No_cond};
  op_table[0x66] = {&Cpu::LD, &Cpu::AM_R_I16, regH, regHL, No_cond};
  op_table[0x67] = {&Cpu::LD, &Cpu::AM_R_R, regH, regA, No_cond};
  op_table[0x68] = {&Cpu::LD, &Cpu::AM_R_R, regL, regB, No_cond};
  op_table[0x69] = {&Cpu::LD, &Cpu::AM_R_R, regL, regC, No_cond};
  op_table[0x6A] = {&Cpu::LD, &Cpu::AM_R_R, regL, regD, No_cond};
  op_table[0x6B] = {&Cpu::LD, &Cpu::AM_R_R, regL, regE, No_cond};
  op_table[0x6C] = {&Cpu::LD, &Cpu::AM_R_R, regL, regH, No_cond};
  op_table[0x6D] = {&Cpu::LD, &Cpu::AM_R_R, regL, regL, No_cond};
  op_table[0x6E] = {&Cpu::LD, &Cpu::AM_R_I16, regL, regHL, No_cond};
  op_table[0x6F] = {&Cpu::LD, &Cpu::AM_R_R, regL, regA, No_cond};

  op_table[0x70] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regB, No_cond};
  op_table[0x71] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regC, No_cond};
  op_table[0x72] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regD, No_cond};
  op_table[0x73] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regE, No_cond};
  op_table[0x74] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regH, No_cond};
  op_table[0x75] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regL, No_cond};
  op_table[0x76] = {&Cpu::HALT, &Cpu::NONE, None, None, No_cond};
  op_table[0x77] = {&Cpu::LD, &Cpu::AM_I16_R, regHL, regA, No_cond};
  op_table[0x78] = {&Cpu::LD, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0x79] = {&Cpu::LD, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0x7A] = {&Cpu::LD, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0x7B] = {&Cpu::LD, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0x7C] = {&Cpu::LD, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0x7D] = {&Cpu::LD, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0x7E] = {&Cpu::LD, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0x7F] = {&Cpu::LD, &Cpu::AM_R_R, regA, regA, No_cond};

  op_table[0x80] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0x81] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0x82] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0x83] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0x84] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0x85] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0x86] = {&Cpu::ADD, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0x87] = {&Cpu::ADD, &Cpu::AM_R_R, regA, regA, No_cond};
  op_table[0x88] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0x89] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0x8A] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0x8B] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0x8C] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0x8D] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0x8E] = {&Cpu::ADC, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0x8F] = {&Cpu::ADC, &Cpu::AM_R_R, regA, regA, No_cond};

  op_table[0x90] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0x91] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0x92] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0x93] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0x94] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0x95] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0x96] = {&Cpu::SUB, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0x97] = {&Cpu::SUB, &Cpu::AM_R_R, regA, regA, No_cond};
  op_table[0x98] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0x99] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0x9A] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0x9B] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0x9C] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0x9D] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0x9E] = {&Cpu::SBC, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0x9F] = {&Cpu::SBC, &Cpu::AM_R_R, regA, regA, No_cond};

  op_table[0xA0] = {&Cpu::AND, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0xA1] = {&Cpu::AND, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0xA2] = {&Cpu::AND, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0xA3] = {&Cpu::AND, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0xA4] = {&Cpu::AND, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0xA5] = {&Cpu::AND, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0xA6] = {&Cpu::AND, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0xA7] = {&Cpu::AND, &Cpu::AM_R_R, regA, regA, No_cond};
  op_table[0xA8] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0xA9] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0xAA] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0xAB] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0xAC] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0xAD] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0xAE] = {&Cpu::XOR, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0xAF] = {&Cpu::XOR, &Cpu::AM_R_R, regA, regA, No_cond};

  op_table[0xB0] = {&Cpu::OR, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0xB1] = {&Cpu::OR, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0xB2] = {&Cpu::OR, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0xB3] = {&Cpu::OR, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0xB4] = {&Cpu::OR, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0xB5] = {&Cpu::OR, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0xB6] = {&Cpu::OR, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0xB7] = {&Cpu::OR, &Cpu::AM_R_R, regA, regA, No_cond};
  op_table[0xB8] = {&Cpu::CP, &Cpu::AM_R_R, regA, regB, No_cond};
  op_table[0xB9] = {&Cpu::CP, &Cpu::AM_R_R, regA, regC, No_cond};
  op_table[0xBA] = {&Cpu::CP, &Cpu::AM_R_R, regA, regD, No_cond};
  op_table[0xBB] = {&Cpu::CP, &Cpu::AM_R_R, regA, regE, No_cond};
  op_table[0xBC] = {&Cpu::CP, &Cpu::AM_R_R, regA, regH, No_cond};
  op_table[0xBD] = {&Cpu::CP, &Cpu::AM_R_R, regA, regL, No_cond};
  op_table[0xBE] = {&Cpu::CP, &Cpu::AM_R_I16, regA, regHL, No_cond};
  op_table[0xBF] = {&Cpu::CP, &Cpu::AM_R_R, regA, regA, No_cond};

  op_table[0xC0] = {&Cpu::RET, &Cpu::NONE, None, None, NZ_cond};
  op_table[0xC1] = {&Cpu::POP, &Cpu::AM_RR, regBC, None, No_cond};
  op_table[0xC2] = {&Cpu::JP, &Cpu::NONE, None, None, NZ_cond};
  op_table[0xC3] = {&Cpu::JP, &Cpu::NONE, None, None, No_cond};
  op_table[0xC4] = {&Cpu::CALL, &Cpu::NONE, None, None, NZ_cond};
  op_table[0xC5] = {&Cpu::PUSH, &Cpu::AM_RR, regBC, None, No_cond};
  op_table[0xC6] = {&Cpu::ADD, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xC7] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};
  op_table[0xC8] = {&Cpu::RET, &Cpu::NONE, None, None, Z_cond};
  op_table[0xC9] = {&Cpu::RET, &Cpu::NONE, None, None, No_cond};
  op_table[0xCA] = {&Cpu::JP, &Cpu::NONE, None, None, Z_cond};
  op_table[0xCC] = {&Cpu::CALL, &Cpu::NONE, None, None, Z_cond};
  op_table[0xCD] = {&Cpu::CALL, &Cpu::NONE, None, None, No_cond};
  op_table[0xCE] = {&Cpu::ADC, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xCF] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};

  op_table[0xD0] = {&Cpu::RET, &Cpu::NONE, None, None, NC_cond};
  op_table[0xD1] = {&Cpu::POP, &Cpu::AM_RR, regDE, None, No_cond};
  op_table[0xD2] = {&Cpu::JP, &Cpu::NONE, None, None, NC_cond};
  op_table[0xD4] = {&Cpu::CALL, &Cpu::NONE, None, None, NC_cond};
  op_table[0xD5] = {&Cpu::PUSH, &Cpu::AM_RR, regDE, None, No_cond};
  op_table[0xD6] = {&Cpu::SUB, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xD7] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};
  op_table[0xD8] = {&Cpu::RET, &Cpu::NONE, None, None, C_cond};
  op_table[0xD9] = {&Cpu::RETI, &Cpu::NONE, None, None, No_cond};
  op_table[0xDA] = {&Cpu::JP, &Cpu::NONE, None, None, C_cond};
  op_table[0xDC] = {&Cpu::CALL, &Cpu::NONE, None, None, C_cond};
  op_table[0xDE] = {&Cpu::SBC, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xDF] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};

  op_table[0xE0] = {&Cpu::LD, &Cpu::AM_FFD8_R, regA, None, No_cond};
  op_table[0xE1] = {&Cpu::POP, &Cpu::AM_RR, regHL, None, No_cond};
  op_table[0xE2] = {&Cpu::LD, &Cpu::AM_FFI8_R, regC, regA, No_cond};
  op_table[0xE5] = {&Cpu::PUSH, &Cpu::AM_RR, regHL, None, No_cond};
  op_table[0xE6] = {&Cpu::AND, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xE7] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};
  op_table[0xE8] = {&Cpu::ADD, &Cpu::AM_RR_NN, regSP, None, No_cond};
  op_table[0xE9] = {&Cpu::JP, &Cpu::AM_RR, regHL, None, No_cond};
  op_table[0xEA] = {&Cpu::LD, &Cpu::AM_a16_R, regA, None, No_cond};
  op_table[0xEE] = {&Cpu::XOR, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xEF] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};

  op_table[0xF0] = {&Cpu::LD, &Cpu::AM_R_FFD8, regA, None, No_cond};
  op_table[0xF1] = {&Cpu::POP, &Cpu::AM_RR, regAF, None, No_cond};
  op_table[0xF2] = {&Cpu::LD, &Cpu::AM_R_FFI8, regA, regC, No_cond};
  op_table[0xF3] = {&Cpu::DI, &Cpu::NONE, None, None, No_cond};
  op_table[0xF5] = {&Cpu::PUSH, &Cpu::AM_RR, regAF, None, No_cond};
  op_table[0xF6] = {&Cpu::OR, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xF7] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};
  op_table[0xF8] = {&Cpu::LD, &Cpu::AM_HL_SP_S8, regHL, regSP, No_cond};
  op_table[0xF9] = {&Cpu::LD, &Cpu::AM_RR_RR, regSP, regHL, No_cond};
  op_table[0xFA] = {&Cpu::LD, &Cpu::AM_R_a16, regA, None, No_cond};
  op_table[0xFB] = {&Cpu::EI, &Cpu::NONE, None, None, No_cond};
  op_table[0xFE] = {&Cpu::CP, &Cpu::AM_R_D8, regA, None, No_cond};
  op_table[0xFF] = {&Cpu::RST, &Cpu::NONE, None, None, No_cond};
};

void Cpu::connectBus(Bus *bus) { Cpu::bus = bus; }

void Cpu::fetch_instruction() { Cpu::opcode = Cpu::read(Cpu::pc); }

void Cpu::fetch() {}
void Cpu::decode() {}

void Cpu::write(uint8_t data, uint16_t address) {
  Cpu::bus->write(data, address);
}
uint8_t Cpu::read(uint16_t address) { return Cpu::bus->read(address); }

uint16_t Cpu::get_reg(reg_code type) {
  switch (type) {
  case regA:
    return this->af.Reg8.higher;
  case regF:
    return this->af.Reg8.lower;
  case regB:
    return this->bc.Reg8.higher;
  case regC:
    return this->bc.Reg8.lower;
  case regD:
    return this->de.Reg8.higher;
  case regE:
    return this->de.Reg8.lower;
  case regH:
    return this->hl.Reg8.higher;
  case regL:
    return this->hl.Reg8.lower;
  default:
    return 0x00;
  }
}

void Cpu::set_reg(reg_code type, uint16_t data) {
  switch (type) {
  case regA:
    af.Reg8.higher = data;
    break;
  case regF:
    af.Reg8.lower = data;
    break;
  case regB:
    bc.Reg8.higher = data;
    break;
  case regC:
    bc.Reg8.lower = data;
    break;
  case regD:
    de.Reg8.higher = data;
    break;
  case regE:
    de.Reg8.lower = data;
    break;
  case regH:
    hl.Reg8.higher = data;
    break;
  case regL:
    hl.Reg8.lower = data;
    break;
  default:
    break;
  }
}

uint8_t Cpu::get_flag(flags flag) { return (af.Reg8.lower & flag) ? 1 : 0; }

void Cpu::change_flag(flags flag) { af.Reg8.lower ^= flag; }

void Cpu::clear_flag(flags flag) { af.Reg8.lower &= ~flag; }

void Cpu::set_flag(flags flag) { af.Reg8.lower |= flag; }

// Address mode implementations
void Cpu::NONE() {}

void Cpu::AM_R() {
  this->operand1 = get_reg(this->act_instruction->reg1);
  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_I16() {
  exec_cycle(1);
  this->operand1 = read(get_reg(this->act_instruction->reg1));
  this->address = get_reg(this->act_instruction->reg1);

  address_type = to_memory;
  bit16 = false;
}

void Cpu::AM_R_R() {
  this->operand1 = get_reg(this->act_instruction->reg1);
  this->operand2 = get_reg(this->act_instruction->reg2);
  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_R_D8() {
  this->operand1 = get_reg(this->act_instruction->reg1);
  this->operand2 = read(this->pc + 1);
  this->exec_cycle(1);
  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_R_I16() {
  this->operand1 = get_reg(this->act_instruction->reg1);
  this->operand2 = read(get_reg(this->act_instruction->reg2));

  if (this->act_instruction->reg2 == regHLaddI) {
    hl.reg++;
  }
  if (this->act_instruction->reg2 == regHLaddD) {
    hl.reg--;
  }

  this->exec_cycle(1);
  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_I16_R() {
  // this->operand1 = read(get_reg(this->act_instruction->reg1));
  this->operand2 = get_reg(this->act_instruction->reg2);

  this->address = get_reg(this->act_instruction->reg1);

  /*if (this->act_instruction->reg1 == regHLaddI) {
    hl.reg++;
  }
  if (this->act_instruction->reg1 == regHLaddD) {
    hl.reg--;
  }*/

  // this->exec_cycle(1);
  address_type = to_memory;
  bit16 = false;
}

void Cpu::AM_I16_D8() {
  // this->operand1 = read(get_reg(this->act_instruction->reg1));
  exec_cycle(1);
  this->operand2 = read(pc++);

  this->address = get_reg(this->act_instruction->reg1);

  address_type = to_memory;
  bit16 = false;
}

void Cpu::AM_R_a16() {
  exec_cycle(1);
  uint8_t lower_byte = read(pc++);
  exec_cycle(1);
  uint8_t higher_byte = read(pc++);
  exec_cycle(1);
  this->operand2 = read((higher_byte << 8) | lower_byte);

  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_a16_R() {
  this->operand2 = get_reg(this->act_instruction->reg1);
  exec_cycle(1);
  uint8_t lower_byte = read(pc++);
  exec_cycle(1);
  uint8_t higher_byte = read(pc++);

  address = (higher_byte << 8) | lower_byte;
  address_type = to_memory;
  bit16 = false;
}

void Cpu::AM_FFD8_R() {
  this->operand2 = get_reg(this->act_instruction->reg1);
  exec_cycle(1);
  uint8_t lower_byte = read(pc++);
  this->address = 0xFF00 | lower_byte;
  address_type = to_memory;
  bit16 = false;
}

void Cpu::AM_R_FFD8() {
  exec_cycle(1);
  uint8_t lower_byte = read(pc++);
  exec_cycle(1);
  this->operand2 = read(0xFF00 | lower_byte);
  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_FFI8_R() {
  this->operand2 = get_reg(this->act_instruction->reg2);
  this->address = (0xFF00 | get_reg(this->act_instruction->reg1));

  address_type = to_memory;
  bit16 = false;
}

void Cpu::AM_R_FFI8() {
  uint8_t lower_byte = get_reg(this->act_instruction->reg2);
  exec_cycle(1);
  this->operand2 = read(0xFF00 | lower_byte);

  address_type = to_reg;
  bit16 = false;
}

void Cpu::AM_RR() {
  this->operand1 = (get_reg(this->act_instruction->reg1));
  address_type = to_reg;
  bit16 = true;
}

void Cpu::AM_RR_NN() {
  exec_cycle(1);
  uint8_t lower_byte = read(pc++);
  exec_cycle(1);
  uint8_t higher_byte = read(pc++);
  this->operand2 = (higher_byte << 8) | lower_byte;

  address_type = to_reg;
  bit16 = true;
}

void Cpu::AM_a16_RR() {
  this->operand2 = get_reg(this->act_instruction->reg1);
  exec_cycle(1);
  uint8_t lower_byte = read(pc++);
  exec_cycle(1);
  uint8_t higher_byte = read(pc++);

  this->address = ((higher_byte << 8) | lower_byte);
  address_type = to_memory;
  bit16 = true;
}

void Cpu::AM_RR_RR() {
  this->operand2 = get_reg(this->act_instruction->reg2);
  address_type = to_reg;
  bit16 = true;
}

void Cpu::AM_SP_S8() {
  exec_cycle(1);
  this->operand2 = read(pc++);
  address_type = to_reg;
  bit16 = true;
}

void Cpu::AM_HL_SP_S8() {
  exec_cycle(1);
  int8_t s8 = read(pc++);
  uint32_t result = sp + s8;

  if (result > UINT16_MAX && get_flag(cy) == 0)
    change_flag(cy);

  uint16_t format_result = (uint16_t)result;

  if ((sp & 0x0F) + (s8 & 0x0F) > 0x0F && get_flag(h) == 0)
    change_flag(h);

  this->operand2 = format_result;
  exec_cycle(1);
}

// Instruction implementations
void Cpu::NOP() {}

// Loads data to a register or address in memory
void Cpu::LD() {
  uint16_t data = operand2;

  if (address_type == to_reg) {
    set_reg(this->act_instruction->reg1, data);
  } else {
    if (!bit16) {
      exec_cycle(1);
      write((uint8_t)data, address);
    } else {
      exec_cycle(1);
      write((uint8_t)(data & 0x00FF), address);
      exec_cycle(1);
      write((uint8_t)(data >> 8), address + 1);
    }
  }
}

// Push to stack
void Cpu::PUSH() {
  exec_cycle(1);
  sp--;
  exec_cycle(1);
  write((uint8_t)(operand1 & 0x00FF), sp);
  sp--;
  exec_cycle(1);
  write((uint8_t)(operand1 >> 8), sp);
}

// Pop from stack
void Cpu::POP() {
  exec_cycle(1);
  uint8_t lower_byte = read(sp++);
  exec_cycle(1);
  uint8_t higher_byte = read(sp++);
  set_reg(act_instruction->reg1, (higher_byte << 8) | lower_byte);
}

// Add to register
void Cpu::ADD() {
  if (!bit16) {
    uint16_t result = get_reg(act_instruction->reg1) + operand2;
    if (result > 0xFF)
      set_flag(cy);
    else
      clear_flag(cy);
    if ((get_reg(act_instruction->reg1) & 0x0F) + (operand2 & 0x0F) > 0x0F)
      set_flag(h);
    else
      clear_flag(h);

    if (result == 0)
      set_flag(z);
    else
      clear_flag(z);

    set_reg(act_instruction->reg1, result);
  } else {
    uint16_t low_result =
        (get_reg(act_instruction->reg1) & 0x00FF) + (operand2 & 0x00FF);

    uint16_t high_result =
        (get_reg(act_instruction->reg1) >> 8) + (operand2 >> 8);

    if (low_result > 0xFF)
      set_flag(cy);
    else
      clear_flag(cy);

    if (low_result & 0x0F)
      set_flag(h);
    else
      clear_flag(h);

    exec_cycle(1);

    high_result += get_flag(cy);
    if (high_result > 0xFF)
      set_flag(cy);
    else
      clear_flag(cy);
    if (high_result & 0x0F)
      set_flag(h);
    else
      clear_flag(h);

    clear_flag(z);

    set_reg(act_instruction->reg1, (high_result << 8) | low_result);
  }
  clear_flag(n);
}

// Add with carry to register A
void Cpu::ADC() {
  uint16_t result = af.Reg8.higher + operand2 + get_flag(cy);
  if (result > 0xFF)
    set_flag(cy);
  else
    clear_flag(cy);
  if ((get_reg(act_instruction->reg1) & 0x0F) + (operand2 & 0x0F) > 0x0F)
    set_flag(h);
  else
    clear_flag(h);

  if (result == 0)
    set_flag(z);
  else
    clear_flag(z);

  clear_flag(n);

  af.Reg8.higher = (uint8_t)(result & 0x00FF);
}

// Substracts from A register
void Cpu::SUB() {
  uint16_t result = af.Reg8.higher - operand2;
  if (result > 0xFF)
    set_flag(cy);
  else
    clear_flag(cy);
  if ((get_reg(act_instruction->reg1) & 0x0F) - (operand2 & 0x0F) > 0x0F)
    set_flag(h);
  else
    clear_flag(h);

  if (result == 0)
    set_flag(z);
  else
    clear_flag(z);

  set_flag(n);

  af.Reg8.higher = (uint8_t)(result & 0x00FF);
}

// Substracts from A register with carry
void Cpu::SBC() {
  uint16_t result = af.Reg8.higher - operand2 - get_flag(cy);
  if (result > 0xFF)
    set_flag(cy);
  else
    clear_flag(cy);
  if ((get_reg(act_instruction->reg1) & 0x0F) - (operand2 & 0x0F) > 0x0F)
    set_flag(h);
  else
    clear_flag(h);

  if (result == 0)
    set_flag(z);
  else
    clear_flag(z);

  set_flag(n);

  af.Reg8.higher = (uint8_t)(result & 0x00FF);
}

// Performs And bitwise operation with A register
void Cpu::AND() {
  af.Reg8.higher &= operand2;

  if (af.Reg8.higher == 0)
    set_flag(z);
  else
    clear_flag(z);

  clear_flag(n);
  set_flag(h);
  clear_flag(cy);
}

// Performs Or bitwise operation with A register
void Cpu::OR() {
  af.Reg8.higher |= operand2;

  if (af.Reg8.higher == 0)
    set_flag(z);
  else
    clear_flag(z);

  clear_flag(n);
  clear_flag(h);
  clear_flag(cy);
}

// Performs Xor bitwise operation with A register
void Cpu::XOR() {
  af.Reg8.higher ^= operand2;

  if (af.Reg8.higher == 0)
    set_flag(z);
  else
    clear_flag(z);

  clear_flag(n);
  clear_flag(h);
  clear_flag(cy);
}

// Compares a operand with the A register
void Cpu::CP() {
  uint16_t result = af.Reg8.higher - operand2;

  if (result > 0xFF)
    set_flag(cy);
  else
    clear_flag(cy);
  if ((get_reg(act_instruction->reg1) & 0x0F) - (operand2 & 0x0F) > 0x0F)
    set_flag(h);
  else
    clear_flag(h);

  if (result == 0)
    set_flag(z);
  else
    clear_flag(z);

  set_flag(n);
}

// Increments by 1
void Cpu::INC() {
  if (!bit16) {
    uint16_t result = this->operand1 + 1;

    if ((get_reg(act_instruction->reg1) & 0x0F) - (operand2 & 0x0F) > 0x0F)
      set_flag(h);
    else
      clear_flag(h);

    if (result == 0)
      set_flag(z);
    else
      clear_flag(z);

    clear_flag(n);

    if (address_type == to_reg)
      set_reg(this->act_instruction->reg1, (uint8_t)(result & 0x00FF));
    else {
      exec_cycle(1);
      write((uint8_t)(result & 0x00FF), this->address);
    }

  } else {
    exec_cycle(1);
    set_reg(this->act_instruction->reg1, this->operand1 + 1);
  }
}

// Decrements by 1
void Cpu::DEC() {
  if (!bit16) {
    uint16_t result = this->operand1 - 1;

    if ((get_reg(act_instruction->reg1) & 0x0F) - (operand2 & 0x0F) > 0x0F)
      set_flag(h);
    else
      clear_flag(h);

    if (result == 0)
      set_flag(z);
    else
      clear_flag(z);

    set_flag(n);

    if (address_type == to_reg)
      set_reg(this->act_instruction->reg1, (uint8_t)(result & 0x00FF));
    else {
      exec_cycle(1);
      write((uint8_t)(result & 0x00FF), this->address);
    }

  } else {
    exec_cycle(1);
    set_reg(this->act_instruction->reg1, this->operand1 - 1);
  }
}