#include "apu.h"
#include "bus.h"
#include <cmath>
#include <cstdint>

Apu::Apu() {
  SDL_AudioSpec spec;

  spec.format = SDL_AUDIO_F32;
  spec.channels = 1;
  spec.freq = 44100;

  audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                           &spec, NULL, NULL);

  if (audio_stream) {
    SDL_ResumeAudioStreamDevice(audio_stream);
  }
}
Apu::~Apu() {
  if (audio_stream) {
    SDL_DestroyAudioStream(audio_stream);
  }
}

void Apu::connectBus(Bus *bus) {
  this->bus = bus;
  audio_master = bus->get_address(0xFF26);
  audio_panning = bus->get_address(0xFF25);
  Volume_Vin = bus->get_address(0xFF24);

  waves = bus->get_address(0xFF30);

  pulse1 = (channel_reg *)bus->get_address(0xFF10);
  pulse2 = (channel_reg *)bus->get_address(0xFF15);
  wave = (channel_reg *)bus->get_address(0xFF1A);
  noise = (channel_reg *)bus->get_address(0xFF1F);

  update_channel_reg1();
  update_channel_reg2();
  update_channel_reg3();
  update_channel_reg4();
}

void Apu::step(uint8_t cycles) {
  if ((*audio_master & 0x80) == 0) {
    return;
  }
  this->delta = (1.0f / 1048576.0f) * cycles;
  this->elapsed_time += delta;
  handle_pulse1();
  handle_pulse2();
  handle_wave();
  handle_noise();

  if (elapsed_time >= (1.0f / 44100.0f)) {
    elapsed_time -= (1.0f / 44100.0f);
    send_samples();
  }
}

void Apu::send_samples() {
  if (!audio_stream)
    return;

  float sample1 = 0;
  if (ch1) {
    bool bit = (channel_1.wave_duty >> channel_1.duty_step) & 0x01;
    sample1 = bit ? (float)channel_1.volume / 15.0f
                  : -(float)channel_1.volume / 15.0f;
  }

  float sample2 = 0;
  if (ch2) {
    bool bit = (channel_2.wave_duty >> channel_2.duty_step) & 0x01;
    sample2 = bit ? (float)channel_2.volume / 15.0f
                  : -(float)channel_2.volume / 15.0f;
  }

  float sample3 = 0;
  if (ch3) {
    uint8_t sample_index = channel_3.sample_counter / 2;
    uint8_t nibble_byte = waves[sample_index];
    uint8_t sample = (channel_3.sample_counter % 2 == 0) ? (nibble_byte >> 4)
                                                         : (nibble_byte & 0x0F);

    sample3 = (float)((int16_t)sample - 8) / 8.0f;
    // Apply volume shift: 0: 100%, 1: 50%, 2: 25%, 4: Mute
    if (channel_3.volume == 1)
      sample3 *= 0.5f;
    else if (channel_3.volume == 2)
      sample3 *= 0.25f;
    else if (channel_3.volume == 4)
      sample3 = 0.0f;
  }

  float sample4 = 0;
  if (ch4) {
    uint8_t sample = (channel_4.lfsr & 0x0001) ? 1 : 0;
    sample4 = sample ? (float)channel_4.volume / 15.0f
                     : -(float)channel_4.volume / 15.0f;
  }

  // Mix channels (0.25 gain per channel to avoid clipping)
  mixed[mix_index++] = (sample1 + sample2 + sample3 + sample4) * 0.25f;

  if (mix_index >= 512) {
    SDL_PutAudioStreamData(audio_stream, mixed, sizeof(float) * 512);
    mix_index = 0;
  }
}

uint8_t Apu::get_sample_ch1() {
  if (!ch1) {
    return 0;
  }
  uint8_t sample = (channel_1.wave_duty >> channel_1.duty_step) & 0x01;
  return (sample * channel_1.volume) & 0x0F;
}

uint8_t Apu::get_sample_ch2() {
  if (!ch2) {
    return 0;
  }
  uint8_t sample = (channel_2.wave_duty >> channel_2.duty_step) & 0x01;
  return (sample * channel_2.volume) & 0x0F;
}

void Apu::handle_pulse1() {
  if (!ch1)
    return;
  channel_1.elapsed_time += delta;

  // 1. LENGTH COUNTER (256Hz)
  if (channel_1.length_enable) {
    channel_1.length_timer += delta;
    if (channel_1.length_timer >= (1.0f / 256.0f)) {
      channel_1.length_timer -= (1.0f / 256.0f);
      channel_1.length_counter++;
      if (channel_1.length_counter >= 64) {
        ch1 = false;
        return;
      }
    }
  }

  // 2. SWEEP (128Hz)
  if (channel_1.sweep_pace > 0) {
    channel_1.sweep_timer += delta;
    float sweep_target = (float)channel_1.sweep_pace / 128.0f;
    if (channel_1.sweep_timer >= sweep_target) {
      channel_1.sweep_timer -= sweep_target;

      int16_t freq_offset = channel_1.frequency_Shadow >> channel_1.sweep_step;
      if (channel_1.sweep_direction) { // Decrement
        channel_1.frequency_Shadow -= freq_offset;
      } else { // Increment
        if (channel_1.frequency_Shadow + freq_offset > 2047) {
          channel_1.frequency_Shadow = 2047;
          ch1 = false;
          return;
        }
        channel_1.frequency_Shadow += freq_offset;
      }

      if (channel_1.frequency_Shadow < 0) {
        channel_1.frequency_Shadow = 0;
      }

      // Actualizar frecuencia real
      channel_1.frequency =
          131072.0f / (2048.0f - (float)channel_1.frequency_Shadow);
      channel_1.period = 1.0f / channel_1.frequency;
    }
  }

  // 3. DUTY STEP CYCLE
  channel_1.duty_timer += delta;
  float step_time = channel_1.period / 8.0f;
  if (channel_1.duty_timer >= step_time && step_time > 0) {
    channel_1.duty_timer -= step_time;
    channel_1.duty_step = (channel_1.duty_step + 1) % 8;
  }

  // 4. VOLUME ENVELOPE (64Hz)
  if (channel_1.env_pace > 0) {
    channel_1.env_timer += delta;
    float env_target = (float)channel_1.env_pace / 64.0f;
    if (channel_1.env_timer >= env_target) {
      channel_1.env_timer -= env_target;
      if (channel_1.env_dir == 0 && channel_1.volume > 0) {
        channel_1.volume--;
      } else if (channel_1.env_dir == 1 && channel_1.volume < 15) {
        channel_1.volume++;
      }
    }
  }
}

void Apu::handle_pulse2() {
  if (!ch2)
    return;
  channel_2.elapsed_time += delta;

  // 1. LENGTH COUNTER (256Hz)
  if (channel_2.length_enable) {
    channel_2.length_timer += delta;
    if (channel_2.length_timer >= (1.0f / 256.0f)) {
      channel_2.length_timer -= (1.0f / 256.0f);
      if (channel_2.length_counter < 64) {
        channel_2.length_counter++;
      }
      if (channel_2.length_counter >= 64) {
        ch2 = false;
        return;
      }
    }
  }

  // 2. DUTY STEP CYCLE
  channel_2.duty_timer += delta;
  float step_time = channel_2.period / 8.0f;
  if (channel_2.duty_timer >= step_time && step_time > 0) {
    channel_2.duty_timer -= step_time;
    channel_2.duty_step = (channel_2.duty_step + 1) % 8;
  }

  // 3. VOLUME ENVELOPE (64Hz)
  if (channel_2.env_pace > 0) {
    channel_2.env_timer += delta;
    float env_target = (float)channel_2.env_pace / 64.0f;
    if (channel_2.env_timer >= env_target) {
      channel_2.env_timer -= env_target;
      if (channel_2.env_dir == 0 && channel_2.volume > 0) {
        channel_2.volume--;
      } else if (channel_2.env_dir == 1 && channel_2.volume < 15) {
        channel_2.volume++;
      }
    }
  }
}

void Apu::handle_wave() {
  if (!ch3)
    return;

  channel_3.elapsed_time += delta;

  if (channel_3.length_enable) {
    channel_3.length_timer += delta;
    if (channel_3.length_timer >= (1.0f / 256.0f)) {
      channel_3.length_timer -= (1.0f / 256.0f);
      if (channel_3.length_counter < 256) {
        channel_3.length_counter++;
      }
      if (channel_3.length_counter >= 256) {
        ch3 = false;
        return;
      }
    }
  }

  channel_3.sample_timer += delta;
  float playback_speed = 1.0f / (32.0f * channel_3.frequency);
  if (channel_3.sample_timer >= playback_speed && channel_3.frequency > 0) {
    channel_3.sample_timer -= playback_speed;
    channel_3.sample_counter++;
    if (channel_3.sample_counter >= 32) {
      channel_3.sample_counter = 0;
    }
  }
}

void Apu::handle_noise() {
  if (!ch4)
    return;

  channel_4.elapsed_time += delta;

  if (channel_4.length_enable) {
    channel_4.length_timer += delta;
    if (channel_4.length_timer >= (1.0f / 256.0f)) {
      channel_4.length_timer -= (1.0f / 256.0f);
      if (channel_4.length_counter < 64) {
        channel_4.length_counter++;
      }
      if (channel_4.length_counter >= 64) {
        ch4 = false;
        return;
      }
    }
  }

  if (channel_4.env_pace > 0) {
    channel_4.env_timer += delta;
    float env_target = (float)channel_4.env_pace / 64.0f;
    if (channel_4.env_timer >= env_target) {
      channel_4.env_timer -= env_target;
      if (channel_4.env_dir == 0 && channel_4.volume > 0) {
        channel_4.volume--;
      } else if (channel_4.env_dir == 1 && channel_4.volume < 15) {
        channel_4.volume++;
      }
    }
  }

  channel_4.lfsr_timer += delta;
  if (channel_4.lfsr_timer >= (1.0f / channel_4.lfsr_frequency)) {
    channel_4.lfsr_timer -= (1.0f / channel_4.lfsr_frequency);
    uint8_t xor_val =
        ~((channel_4.lfsr & 0x01) ^ ((channel_4.lfsr >> 1) & 0x01)) & 0x01;

    channel_4.lfsr = (channel_4.lfsr & (~0x8000)) | (xor_val << 15);
    if (channel_4.lfsr_width == 7)
      channel_4.lfsr = (channel_4.lfsr & (~0x0080)) | (xor_val << 7);
    channel_4.lfsr >>= 1;
  }
}

void Apu::update_channel_reg1() {
  // NR14 bit 7 is trigger
  if (pulse1->Nrx4 & 0x80) {
    ch1 = true;
    channel_1.duty_step = 0;
    channel_1.elapsed_time = 0.0f;
    channel_1.duty_timer = 0.0f;
    channel_1.sweep_timer = 0.0f;
    channel_1.length_timer = 0.0f;
    channel_1.env_timer = 0.0f;

    // Reset length counter if needed
    if (channel_1.length_counter == 0)
      channel_1.length_counter = 64;

    // Initial volume
    channel_1.volume = (pulse1->Nrx2 >> 4) & 0x0F;
  }

  // Update duty cycle
  switch ((pulse1->Nrx1 >> 6) & 0x03) {
  case 0x00:
    channel_1.wave_duty = 0x01;
    break; // 12.5%
  case 0x01:
    channel_1.wave_duty = 0x03;
    break; // 25%
  case 0x02:
    channel_1.wave_duty = 0x0F;
    break; // 50%
  case 0x03:
    channel_1.wave_duty = 0xFC;
    break; // 75%
  }

  channel_1.length_enable = (pulse1->Nrx4 >> 6) & 0x01;
  channel_1.length_counter = pulse1->Nrx1 & 0x3F;

  channel_1.env_dir = (pulse1->Nrx2 >> 3) & 0x01;
  channel_1.env_pace = pulse1->Nrx2 & 0x07;

  channel_1.sweep_pace = (pulse1->Nrx0 >> 4) & 0x07;
  channel_1.sweep_direction = (pulse1->Nrx0 >> 3) & 0x01;
  channel_1.sweep_step = pulse1->Nrx0 & 0x07;

  // NR13 is low 8 bits, NR14 bits 0-2 are high 3 bits
  channel_1.frequency_Shadow = ((pulse1->Nrx4 & 0x07) << 8) | pulse1->Nrx3;
  channel_1.frequency =
      131072.0f / (2048.0f - (float)channel_1.frequency_Shadow);
  if (channel_1.frequency > 0)
    channel_1.period = 1.0f / channel_1.frequency;
  else
    channel_1.period = 0;
}

void Apu::update_channel_reg2() {
  // NR24 bit 7 is trigger
  if (pulse2->Nrx4 & 0x80) {
    ch2 = true;
    channel_2.duty_step = 0;
    channel_2.elapsed_time = 0.0f;
    channel_2.duty_timer = 0.0f;
    channel_2.length_timer = 0.0f;
    channel_2.env_timer = 0.0f;

    // Reset length counter if needed
    if (channel_2.length_counter == 0)
      channel_2.length_counter = 64;

    // Initial volume
    channel_2.volume = (pulse2->Nrx2 >> 4) & 0x0F;
  }

  // Update duty cycle
  switch ((pulse2->Nrx1 >> 6) & 0x03) {
  case 0x00:
    channel_2.wave_duty = 0x01;
    break; // 12.5%
  case 0x01:
    channel_2.wave_duty = 0x03;
    break; // 25%
  case 0x02:
    channel_2.wave_duty = 0x0F;
    break; // 50%
  case 0x03:
    channel_2.wave_duty = 0xFC;
    break; // 75%
  }

  channel_2.length_enable = (pulse2->Nrx4 >> 6) & 0x01;
  channel_2.length_counter = pulse2->Nrx1 & 0x3F;

  channel_2.env_dir = (pulse2->Nrx2 >> 3) & 0x01;
  channel_2.env_pace = pulse2->Nrx2 & 0x07;

  // NR23 is low 8 bits, NR24 bits 0-2 are high 3 bits
  channel_2.frequency_Shadow = ((pulse2->Nrx4 & 0x07) << 8) | pulse2->Nrx3;
  channel_2.frequency =
      131072.0f / (2048.0f - (float)channel_2.frequency_Shadow);
  if (channel_2.frequency > 0)
    channel_2.period = 1.0f / channel_2.frequency;
  else
    channel_2.period = 0;
}

void Apu::update_channel_reg3() {
  if (wave->Nrx4 & 0x80) {
    ch3 = true;
    channel_3.elapsed_time = 0.0f;
    channel_3.sample_timer = 0.0f;
    channel_3.length_timer = 0.0f;

    // Reset length counter if needed
    if (channel_3.length_counter == 0)
      channel_3.length_counter = 256;

    // Initial volume
    channel_3.volume = (wave->Nrx2 >> 5) & 0x03;
  }

  switch ((wave->Nrx2 >> 5) & 0x03) {
  case 0x00:
    channel_3.volume = 4;
    break;
  case 0x01:
    channel_3.volume = 0;
    break;
  case 0x02:
    channel_3.volume = 1;
    break;
  case 0x03:
    channel_3.volume = 2;
    break;
  }

  channel_3.length_enable = (wave->Nrx4 >> 6) & 0x01;
  channel_3.length_counter = wave->Nrx1 & 0x3F;

  // NR33 is low 8 bits, NR34 bits 0-2 are high 3 bits
  int16_t frequency_Shadow = (((wave->Nrx4 & 0x07) << 8) | wave->Nrx3) & 0x7FF;
  channel_3.frequency = 65536.0f / (2048.0f - (float)frequency_Shadow);
  if (channel_3.frequency > 0)
    channel_3.period = 1.0f / channel_3.frequency;
  else
    channel_3.period = 0;
}

void Apu::update_channel_reg4() {
  if (noise->Nrx4 & 0x80) {
    ch4 = true;
    channel_4.elapsed_time = 0.0f;
    channel_4.lfsr_timer = 0.0f;
    channel_4.length_timer = 0.0f;
    channel_4.env_timer = 0.0f;

    // Reset length counter if needed
    if (channel_4.length_counter == 0)
      channel_4.length_counter = 64;

    // Initial volume
    channel_4.volume = (noise->Nrx2 >> 4) & 0x0F;
  }

  channel_4.length_enable = (noise->Nrx4 >> 6) & 0x01;
  channel_4.length_counter = noise->Nrx1 & 0x3F;

  channel_4.env_dir = (noise->Nrx2 >> 3) & 0x01;
  channel_4.env_pace = noise->Nrx2 & 0x07;

  channel_4.lfsr = 0x0000;
  channel_4.divider = noise->Nrx3 & 0x07;

  channel_4.lfsr_width = (noise->Nrx3 >> 3) & 0x01;
  channel_4.clock_shift = (noise->Nrx3 >> 4) & 0x0F;
  float divisor = channel_4.divider == 0 ? 0.5f : (float)channel_4.divider;
  channel_4.lfsr_frequency =
      262144.0f / (divisor * (pow(2.0, channel_4.clock_shift)));
}
