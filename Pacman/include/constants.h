#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL2/SDL.h>

#define MAP_ROWS     31
#define MAP_COLS     28
#define TILE_SIZE    16
#define WINDOW_W     (MAP_COLS * TILE_SIZE)
#define WINDOW_H     (MAP_ROWS * TILE_SIZE)

#define TILE_EMPTY        ' '
#define TILE_WALL         '#'
#define TILE_PELLET       '.'
#define TILE_POWER_PELLET 'O'
#define TILE_DOOR         '-'
#define TILE_TUNNEL       'T'
#define TILE_FRUIT        'F'

#define PTS_DOT          10
#define PTS_PELLET       50
#define PTS_POWER_PELLET 500
#define PTS_GHOST_BASE   200
#define PTS_CHERRY       100
#define PTS_STRAWBERRY   300
#define PTS_ORANGE       500
#define PTS_APPLE        700
#define PTS_MELON        1000
#define PTS_GALAXIAN     2000
#define PTS_BELL         3000
#define PTS_KEY          5000
#define EXTRA_LIFE_SCORE 10000

#define SPEED_PACMAN         9.5f
#define SPEED_PACMAN_EATING  8.5f
#define SPEED_GHOST          7.5f
#define SPEED_GHOST_TUNNEL   3.75f
#define SPEED_GHOST_FRIGHT   4.0f
#define SPEED_GHOST_DEAD     12.0f

#define FRIGHTENED_DURATION  6000
#define FRIGHTENED_FLASH     2000
#define FRUIT_DURATION       10000
#define FRUIT_SPAWN_1        70
#define FRUIT_SPAWN_2        170

#define CLYDE_THRESHOLD  8
#define PINKY_OFFSET     4

#define FRIGHTENED_DURATION_LVL(lvl) \
    ((Uint32)((lvl) >= 17 ? 1000 : (6000 - (lvl) * 300)))

#define DIST_SQ(ax, ay, bx, by) \
    (((ax)-(bx))*((ax)-(bx)) + ((ay)-(by))*((ay)-(by)))

#define WRAP_COL(c) (((c) % MAP_COLS + MAP_COLS) % MAP_COLS)

#endif /* CONSTANTS_H */