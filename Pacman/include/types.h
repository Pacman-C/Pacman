#ifndef TYPES_H
#define TYPES_H

typedef enum {
    DIR_NONE  = -1,
    DIR_UP    =  0,
    DIR_DOWN  =  1,
    DIR_LEFT  =  2,
    DIR_RIGHT =  3
} Direction;

static const int DX[] = { 0,  0, -1,  1};
static const int DY[] = {-1,  1,  0,  0};

typedef enum {
    STATE_MENU,
    STATE_READY,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_PACMAN_DEAD,
    STATE_GAMEOVER,
    STATE_TITLE,
    STATE_WIN,
    STATE_LEVEL_TRANSITION
} GameState;

typedef enum {
    GHOST_SCATTER,
    GHOST_CHASE,
    GHOST_FRIGHTENED,
    GHOST_DEAD,
    GHOST_PEN,
    GHOST_LEAVING
} GhostMode;

typedef enum {
    BLINKY = 0,
    PINKY  = 1,
    INKY   = 2,
    CLYDE  = 3,
    GHOST_COUNT
} GhostId;

typedef enum {
    FRUIT_CHERRY,
    FRUIT_STRAWBERRY,
    FRUIT_ORANGE,
    FRUIT_APPLE,
    FRUIT_MELON,
    FRUIT_GALAXIAN,
    FRUIT_BELL,
    FRUIT_KEY
} FruitType;

#endif /* TYPES_H */