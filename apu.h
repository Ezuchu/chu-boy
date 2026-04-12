#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
class Bus;

struct channel_reg {
  uint8_t Nrx0;
  uint8_t Nrx1;
  uint8_t Nrx2;
  uint8_t Nrx3;
  uint8_t Nrx4;
};

class Apu {
  SDL_AudioStream *audio_stream = nullptr;
  Bus *bus;
  uint8_t DIV;
  uint16_t APU_DIV = 0x0;

  float mixed[1024];
  uint16_t mix_index = 0;

  float elapsed_time = 0.0f;
  float delta = 0.0f;

  uint8_t *audio_master = nullptr;
  uint8_t *audio_panning = nullptr;
  uint8_t *Volume_Vin = nullptr;

  uint8_t *NR10 = nullptr;
  uint8_t *NR30 = nullptr;

  uint8_t *waves = nullptr;

  channel_reg *pulse1;
  channel_reg *pulse2;
  channel_reg *wave;
  channel_reg *noise;

  bool ch1 = false, ch2 = false, ch3 = false, ch4 = false;

  struct {
    float elapsed_time = 0.0f;

    int16_t frequency_Shadow;

    int16_t period_divider;
    float frequency;
    float period;

    uint8_t sweep_pace;
    bool sweep_direction;
    uint8_t sweep_step;
    float sweep_timer;

    uint8_t wave_duty;
    uint8_t length_counter;
    uint8_t duty_step;
    float duty_timer;
    float length_timer;

    uint8_t volume;
    uint8_t env_dir;
    uint8_t env_pace;
    float env_timer;

    bool length_enable;
  } channel_1;

  struct {
    float elapsed_time = 0.0f;

    int16_t frequency_Shadow;

    int16_t period_divider;
    float frequency;
    float period;

    uint8_t wave_duty;
    uint8_t length_counter;
    uint8_t duty_step;
    float duty_timer;
    float length_timer;

    uint8_t volume;
    uint8_t env_dir;
    uint8_t env_pace;
    float env_timer;

    bool length_enable;
  } channel_2;

  struct {
    float elapsed_time = 0.0f;

    uint16_t length_counter;
    float length_timer;

    uint8_t volume;

    int16_t period_divider;
    float frequency;
    float period;

    float sample_frequency;
    float sample_timer;
    uint8_t sample_counter = 0;

    bool length_enable;
  } channel_3;

  struct {
    float elapsed_time = 0.0f;

    uint16_t length_counter;
    float length_timer;

    uint8_t volume;
    uint8_t env_dir;
    uint8_t env_pace;
    float env_timer;

    uint16_t lfsr = 0x0000;
    uint8_t clock_shift;
    uint8_t lfsr_width;
    float lfsr_frequency;
    float divider;
    float frequency;
    float lfsr_timer = 0.0f;

    bool length_enable;
  } channel_4;

  void handle_pulse1();
  void handle_pulse2();
  void handle_wave();
  void handle_noise();

public:
  Apu();
  ~Apu();

  void connectBus(Bus *bus);
  void step(uint8_t cycles);

  void update_channel_reg1();
  void update_channel_reg2();
  void update_channel_reg3();
  void update_channel_reg4();

  void send_samples();

  uint8_t get_sample_ch1();
  uint8_t get_sample_ch2();
  uint8_t get_sample_ch3();
  uint8_t get_sample_ch4();
};