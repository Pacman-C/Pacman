#include "../include/audio.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100

static SDL_AudioDeviceID audio_device = 0;
static int audio_enabled = 0;

typedef struct {
    Uint8 *buffer;
    Uint32 length;
} Sound;

static Sound sounds[SOUND_COUNT];
static SDL_AudioSpec audio_spec;

/* =========================
   UTIL : sine wave U8 FIX
   ========================= */
static Uint8 encode_sample(float sample)
{
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;
    return (Uint8)(128 + sample * 127);
}

static Uint8* generate_sine_wave(int frequency, int duration_ms, int *out_len)
{
    int num_samples = (SAMPLE_RATE * duration_ms) / 1000;
    Uint8 *buffer = malloc(num_samples);

    for (int i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float sample = sinf(2.0f * M_PI * frequency * t);
        buffer[i] = encode_sample(sample);
    }

    *out_len = num_samples;
    return buffer;
}

/* =========================
   SFX INIT
   ========================= */

static void sound_chomp_init(void)
{
    int len1, len2;

    Uint8 *p1 = generate_sine_wave(589, 50, &len1);
    Uint8 *p2 = generate_sine_wave(659, 50, &len2);

    sounds[SOUND_CHOMP].length = len1 + len2;
    sounds[SOUND_CHOMP].buffer = malloc(len1 + len2);

    memcpy(sounds[SOUND_CHOMP].buffer, p1, len1);
    memcpy(sounds[SOUND_CHOMP].buffer + len1, p2, len2);

    free(p1);
    free(p2);
}

static void sound_power_up_init(void)
{
    int len = (SAMPLE_RATE * 400) / 1000;
    sounds[SOUND_POWER_UP].buffer = malloc(len);
    sounds[SOUND_POWER_UP].length = len;

    int freqs[] = {392, 440, 494, 523, 587, 659, 784};
    int steps = 7;
    int chunk = len / steps;

    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < chunk; j++) {
            float t = (float)j / SAMPLE_RATE;
            float sample = sinf(2.0f * M_PI * freqs[i] * t);
            sounds[SOUND_POWER_UP].buffer[i * chunk + j] = encode_sample(sample);
        }
    }
}

static void sound_siren_init(void)
{
    int len = (SAMPLE_RATE * 2000) / 1000;
    sounds[SOUND_SIREN].buffer = malloc(len);
    sounds[SOUND_SIREN].length = len;

    int f1 = 262, f2 = 330;

    for (int i = 0; i < len; i++) {
        float t = (float)i / SAMPLE_RATE;
        int freq = ((int)(t * 2) % 2) ? f1 : f2;

        float sample = sinf(2.0f * M_PI * freq * t) * 0.7f;
        sounds[SOUND_SIREN].buffer[i] = encode_sample(sample);
    }
}

static void sound_ghost_eaten_init(void)
{
    int len = (SAMPLE_RATE * 300) / 1000;
    sounds[SOUND_GHOST_EATEN].buffer = malloc(len);
    sounds[SOUND_GHOST_EATEN].length = len;

    for (int i = 0; i < len; i++) {
        float t = (float)i / SAMPLE_RATE;
        float p = (float)i / len;

        int freq = 800 - (int)(600 * p);
        float sample = sinf(2.0f * M_PI * freq * t);

        sounds[SOUND_GHOST_EATEN].buffer[i] = encode_sample(sample);
    }
}

static void sound_pacman_death_init(void)
{
    int len = (SAMPLE_RATE * 800) / 1000;
    sounds[SOUND_PACMAN_DEATH].buffer = malloc(len);
    sounds[SOUND_PACMAN_DEATH].length = len;

    for (int i = 0; i < len; i++) {
        float t = (float)i / SAMPLE_RATE;
        float p = (float)i / len;

        float freq = 400 - 350 * p;
        float sample = sinf(2.0f * M_PI * freq * t) * (1.0f - p);

        sounds[SOUND_PACMAN_DEATH].buffer[i] = encode_sample(sample);
    }
}

static void sound_intermission_init(void)
{
    int len = (SAMPLE_RATE * 600) / 1000;
    sounds[SOUND_INTERMISSION].buffer = malloc(len);
    sounds[SOUND_INTERMISSION].length = len;

    int notes[] = {659, 587, 523};

    int chunk = len / 3;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < chunk; j++) {
            float t = (float)j / SAMPLE_RATE;
            float sample = sinf(2.0f * M_PI * notes[i] * t);
            sounds[SOUND_INTERMISSION].buffer[i * chunk + j] = encode_sample(sample);
        }
    }
}

static void sound_level_complete_init(void)
{
    int len = (SAMPLE_RATE * 800) / 1000;
    sounds[SOUND_LEVEL_COMPLETE].buffer = malloc(len);
    sounds[SOUND_LEVEL_COMPLETE].length = len;

    int freqs[] = {523, 587, 659, 784};
    int chunk = len / 4;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < chunk; j++) {
            float t = (float)j / SAMPLE_RATE;
            float sample = sinf(2.0f * M_PI * freqs[i] * t);
            sounds[SOUND_LEVEL_COMPLETE].buffer[i * chunk + j] = encode_sample(sample);
        }
    }
}

static void sound_game_over_init(void)
{
    int len = (SAMPLE_RATE * 1500) / 1000;
    sounds[SOUND_GAME_OVER].buffer = malloc(len);
    sounds[SOUND_GAME_OVER].length = len;

    for (int i = 0; i < len; i++) {
        float t = (float)i / SAMPLE_RATE;
        float p = (float)i / len;

        float freq = 400 - 300 * p;
        float sample = sinf(2.0f * M_PI * freq * t) * (1.0f - p);

        sounds[SOUND_GAME_OVER].buffer[i] = encode_sample(sample);
    }
}

/* =========================
   API
   ========================= */

void audio_init(void)
{
    SDL_zero(audio_spec);
    audio_spec.freq = SAMPLE_RATE;
    audio_spec.format = AUDIO_U8;
    audio_spec.channels = 1;
    audio_spec.samples = 2048;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);

    if (audio_device == 0) {
        fprintf(stderr, "Audio init failed: %s\n", SDL_GetError());
        return;
    }

    audio_enabled = 1;

    sound_chomp_init();
    sound_power_up_init();
    sound_siren_init();
    sound_ghost_eaten_init();
    sound_pacman_death_init();
    sound_intermission_init();
    sound_level_complete_init();
    sound_game_over_init();
}

void audio_play(SoundType sound)
{
    if (!audio_enabled || sound >= SOUND_COUNT) return;

    Sound *s = &sounds[sound];
    if (!s->buffer || s->length == 0) return;

    SDL_ClearQueuedAudio(audio_device);

    SDL_QueueAudio(audio_device, s->buffer, s->length);
    SDL_PauseAudioDevice(audio_device, 0);
}

void audio_stop_siren(void)
{
    if (!audio_enabled) return;
    SDL_ClearQueuedAudio(audio_device);
}

void audio_quit(void)
{
    if (audio_device) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }

    for (int i = 0; i < SOUND_COUNT; i++) {
        free(sounds[i].buffer);
        sounds[i].buffer = NULL;
    }
}