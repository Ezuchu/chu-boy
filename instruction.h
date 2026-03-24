#pragma once
#include <cstdint>

class Cpu;

enum reg_code {
  None,
  regA,
  regF,
  regB,
  regC,
  regD,
  regE,
  regH,
  regL,
  regHLadd,
  regHLaddI,
  regHLaddD,
  regAF,
  regBC,
  regDE,
  regHL,
  regPC,
  regSP
};

enum address_mode {
  AM_None,
  AM_r,
  AM_r_r,
  AM_r_d8,
  AM_r_i16,
  AM_i16_r,
  AM_i16_d8,
  AM_r_a16,
  AM_a16_r,
  AM_FFd8_r,
  AM_r_FFd8,
  AM_FFi8_r,
  AM_r_FFi8,

  AM_rr,
  AM_rr_nn,
  AM_a16_rr,
  AM_rr_rr,
  AM_SPs8,
  AM_HL_SPs8

};

enum flag_cond { No_cond, NC_cond, NZ_cond, C_cond, Z_cond };

struct instruction {
  void (Cpu::*operate)(void) = nullptr;
  void (Cpu::*mode)(void) = nullptr;
  reg_code reg1 = None;
  reg_code reg2 = None;
  flag_cond condition = No_cond;
};