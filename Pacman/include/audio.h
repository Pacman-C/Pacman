#ifndef AUDIO_H
#define AUDIO_H

typedef enum {
    SOUND_CHOMP,
    SOUND_POWER_UP,
    SOUND_SIREN,
    SOUND_GHOST_EATEN,
    SOUND_PACMAN_DEATH,
    SOUND_INTERMISSION,
    SOUND_LEVEL_COMPLETE,
    SOUND_GAME_OVER,
    SOUND_COUNT
} SoundType;

void audio_init(void);
void audio_play(SoundType sound);
void audio_stop_siren(void);
void audio_quit(void);

#endif // AUDIO_H