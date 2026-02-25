#ifndef BASE_H
#define BASE_H

#include <SDL2/SDL.h>

/* ── Dimensions ─────────────────────────────── */
#define MAP_ROWS     31
#define MAP_COLS     28
#define TILE_SIZE    16          // pixels par tuile

#define WINDOW_W     (MAP_COLS * TILE_SIZE)
#define WINDOW_H     (MAP_ROWS * TILE_SIZE)

/* ── Tuiles ──────────────────────────────────── */
#define TILE_EMPTY   ' '
#define TILE_WALL    '#'
#define TILE_DOT     '.'
#define TILE_PELLET  'O'   // super gomme
#define TILE_DOOR    '-'   // porte cage fantômes

/* ── Directions ──────────────────────────────── */
typedef enum {
    DIR_NONE  = 0,
    DIR_UP    = 1,
    DIR_DOWN  = 2,
    DIR_LEFT  = 3,
    DIR_RIGHT = 4
} Direction;

/* ── États de jeu ────────────────────────────── */
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER,
    STATE_WIN
} GameState;

/* ── États fantôme ───────────────────────────── */
typedef enum {
    GHOST_SCATTER,
    GHOST_CHASE,
    GHOST_FRIGHTENED,
    GHOST_DEAD
} GhostMode;

/* ── Identifiants fantômes ───────────────────── */
typedef enum {
    BLINKY = 0,
    PINKY  = 1,
    INKY   = 2,
    CLYDE  = 3,
    GHOST_COUNT
} GhostId;

/* ── Structure carte ─────────────────────────── */
typedef struct {
    char  grid[MAP_ROWS][MAP_COLS];
    int   dot_count;        // gommes restantes
    int   total_dots;
} Map;

/* ── Entité générique (joueur & fantômes) ────── */
typedef struct {
    int x, y;               // position en tuiles
    int px, py;             // position en pixels (pour interpolation)
    Direction dir;
    Direction next_dir;
    float speed;            // tuiles/seconde
} Entity;

/* ── Fantôme ─────────────────────────────────── */
typedef struct {
    Entity    entity;
    GhostId   id;
    GhostMode mode;
    int       scatter_x, scatter_y;   // coin cible scatter
    Uint32    mode_timer;             // SDL_GetTicks() référence
    int       is_alive;
} Ghost;

/* ── Joueur ──────────────────────────────────── */
typedef struct {
    Entity  entity;
    int     lives;
    int     score;
    int     anim_frame;     // 0-3 pour animation bouche
    Uint32  power_timer;    // durée power pellet (ms)
    int     is_powered;
} Player;

/* ── État global du jeu ──────────────────────── */
typedef struct {
    Map       map;
    Player    player;
    Ghost     ghosts[GHOST_COUNT];
    GameState state;
    int       level;
    Uint32    last_tick;
} Game;

#endif BASE_H 