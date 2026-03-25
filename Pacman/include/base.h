#ifndef BASE_H
#define BASE_H
 
#include <SDL2/SDL.h>
 
/* ── Dimensions ─────────────────────────────── */
#define MAP_ROWS     31
#define MAP_COLS     28
#define TILE_SIZE    16
 
#define WINDOW_W     (MAP_COLS * TILE_SIZE)
#define WINDOW_H     (MAP_ROWS * TILE_SIZE)
 
/* ── Tuiles ──────────────────────────────────── */
#define TILE_EMPTY        ' '
#define TILE_WALL         '#'
#define TILE_PELLET       '.'   // petite pastille
#define TILE_SUPER_PELLET 'O'   // grosse pastille (rend les fantômes bleus)
#define TILE_DOOR         '-'   // porte de la cage
#define TILE_TUNNEL       'T'   // passage qui TP entre gauche/droite
 
/* ── Points ──────────────────────────────────── */
#define PTS_PELLET        10
#define PTS_SUPER_PELLET  50
#define PTS_GHOST_BASE    200   // x2 à chaque fantôme consécutif
#define PTS_CHERRY        100   // cerise niveau 1
#define EXTRA_LIFE_SCORE  10000
 
/* ── Vitesses (tuiles/seconde) ───────────────── */
#define SPEED_PACMAN         9.5f
#define SPEED_PACMAN_EATING  8.5f
#define SPEED_GHOST          7.5f
#define SPEED_GHOST_TUNNEL   3.75f
#define SPEED_GHOST_FRIGHT   4.0f
#define SPEED_GHOST_DEAD     12.0f
 
/* ── Timers frightened (ms) ──────────────────── */
#define FRIGHTENED_DURATION  6000
#define FRIGHTENED_FLASH     2000   // clignotement avant la fin
 
/* ── Seuils ──────────────────────────────────── */
#define CLYDE_THRESHOLD  8   // distance en tuiles
#define PINKY_OFFSET     4   // cases devant Pac-Man
 
/* ── Directions ──────────────────────────────── */
typedef enum {
    DIR_NONE  = -1,
    DIR_UP    =  0,
    DIR_DOWN  =  1,
    DIR_LEFT  =  2,
    DIR_RIGHT =  3
} Direction;
 
/* ── États de jeu ────────────────────────────── */
typedef enum {
    STATE_MENU,
    STATE_READY,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_PACMAN_DEAD,
    STATE_GAMEOVER,
    STATE_WIN
} GameState;
 
/* ── États fantôme ───────────────────────────── */
typedef enum {
    GHOST_SCATTER,
    GHOST_CHASE,
    GHOST_FRIGHTENED,
    GHOST_DEAD,
    GHOST_PEN,
    GHOST_LEAVING
} GhostMode;
 
/* ── Identifiants fantômes ───────────────────── */
typedef enum {
    BLINKY = 0,
    PINKY  = 1,
    INKY   = 2,
    CLYDE  = 3,
    GHOST_COUNT
} GhostId;
 
/* ── Carte ───────────────────────────────────── */
typedef struct {
    char grid[MAP_ROWS][MAP_COLS];
    int  pellet_count;    // pastilles restantes (petites + grosses)
    int  total_pellets;   // total au début du niveau
} Map;
 
/* ── Entité générique ────────────────────────── */
typedef struct {
    int       x, y; // La position logique (en indices de tableau : ex. ligne 23, colonne 14).
    int       px, py; // La position pixel pour le rendu (ex. 368px, 496px).
    Direction dir; // Direction actuelle de déplacement (DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT)
    Direction next_dir; // Direction que le joueur a input mais pas encore appliquée (pour les virages serrés)
    float     speed;
} Entity;
 
/* ── Fantôme ─────────────────────────────────── */
typedef struct {
    Entity    entity;
    GhostId   id;
    GhostMode mode;
    GhostMode mode_before_fright;
    int       scatter_x, scatter_y;
    int       target_x, target_y;
    Uint32    mode_timer;
    int       is_alive;
} Ghost;
 
/* ── Joueur ──────────────────────────────────── */
typedef struct {
    Entity  entity;
    int     lives;
    int     score;
    int     anim_frame;
    Uint32  power_timer;
    int     is_powered;
} Player;
 
/* ── État global du jeu ──────────────────────── */
typedef struct {
    Map       map;
    Player    player;
    Ghost     ghosts[GHOST_COUNT];
    GameState state;
    int       level;
    int       high_score;
    int       ghosts_eaten_combo;
    Uint32    frightened_start;
    Uint32    last_tick;
} Game;
 
/* ── Macros utilitaires ──────────────────────── */
#define DIST_SQ(ax, ay, bx, by) \
    (((ax)-(bx))*((ax)-(bx)) + ((ay)-(by))*((ay)-(by)))
 
#define WRAP_COL(c) (((c) % MAP_COLS + MAP_COLS) % MAP_COLS)
 
#endif /* BASE_H */