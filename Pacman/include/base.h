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
#define TILE_EMPTY   ' '
#define TILE_WALL    '#'
#define TILE_PELLET     '.' 
#define TILE_POWER_PELLET  'O'
#define TILE_DOOR    '-' // porte de la cage
#define TILE_TUNNEL  'T'    // passage qui TP entre les côtés gauche/droite
 
/* ── Points ──────────────────────────────────── */
#define PTS_DOT          10
#define PTS_PELLET       50
#define PTS_POWER_PELLET 500
#define PTS_GHOST_BASE   200    // x2 à chaque fantôme consécutif
#define PTS_CHERRY       100
#define PTS_STRAWBERRY   300
#define PTS_ORANGE       500
#define PTS_APPLE        700
#define PTS_MELON        1000
#define PTS_GALAXIAN     2000
#define PTS_BELL         3000
#define PTS_KEY          5000
#define EXTRA_LIFE_SCORE 10000
 
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

/* ── Fruits ──────────────────────────────────── */
#define TILE_FRUIT 'F'
#define FRUIT_DURATION 10000    // 10 secondes
#define FRUIT_SPAWN_1  70       // première apparition à 70 pastilles mangées
#define FRUIT_SPAWN_2  170      // deuxième apparition

// Dans base.h
#define FRIGHTENED_DURATION_LVL(lvl) \
    ((Uint32)((lvl) >= 17 ? 1000 : (6000 - (lvl) * 300)))
 
/* ── Directions ──────────────────────────────── */
typedef enum {
    DIR_NONE  = -1,
    DIR_UP    =  0,
    DIR_DOWN  =  1,
    DIR_LEFT  =  2,
    DIR_RIGHT =  3
} Direction;

static const int DX[] = { 0,  0, -1,  1};
static const int DY[] = {-1,  1,  0,  0};
 
/* ── États de jeu ────────────────────────────── */
typedef enum {
    STATE_MENU,
    STATE_READY,        // écran "READY!" avant départ
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_PACMAN_DEAD,  // animation mort en cours
    STATE_GAMEOVER,
    STATE_WIN
} GameState;
 
/* ── États fantôme ───────────────────────────── */
typedef enum {
    GHOST_SCATTER,
    GHOST_CHASE,
    GHOST_FRIGHTENED,
    GHOST_DEAD,         // yeux qui rentrent au pen
    GHOST_PEN,          // encore dans la cage
    GHOST_LEAVING       // en train de sortir
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
    int  pellet_count;     // gommes restantes
    int  total_pellets;     // pour le score et les bonus
} Map;
 
/* ── Entité générique ────────────────────────── */
typedef struct {
    int       x, y;         // position en tuiles
    float       px, py;       // position en pixels (interpolation)
    Direction dir;
    Direction next_dir;     // buffering input (anticipation virage)
    float     speed;        // tuiles/seconde
} Entity;
 
/* ── Fantôme ─────────────────────────────────── */
typedef struct {
    Entity    entity;
    GhostId   id;
    GhostMode mode;
    GhostMode mode_before_fright;   // restaurer après frightened
    int       scatter_x, scatter_y; // coin de patrouille
    int       target_x, target_y;  // case cible courante (BFS/debug)
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
 
/* ── État global du jeu ──────────────────────── */
typedef struct {
    Map       map;
    Player    player;
    Ghost     ghosts[GHOST_COUNT];
    GameState state;
    FruitType fruit_type;

    int       level;
    int       high_score;
    int       ghosts_eaten_combo;   // nb fantômes mangés ce frightened (pour x2)
    Uint32    frightened_start;     // SDL_GetTicks() quand frightened a commencé
    Uint32    death_start;          // SDL_GetTicks() quand mort a commencé (animation)

    Uint32    last_tick;
    int death_reset_done;
    int    scatter_chase_index;
    Uint32 scatter_chase_timer;

    // Fruits
    int    fruit_x, fruit_y;        
    int    fruit_active;           
    Uint32 fruit_timer;            
    int    fruit_spawn_count;   
    
    int ghost_score_visible;
    int ghost_score_value;

    int ghost_score_x;
    int ghost_score_y;

    Uint32 ghost_score_timer;
    int extra_life_earned;
} Game;


/* ── Macros utilitaires ──────────────────────── */
#define DIST_SQ(ax, ay, bx, by) \
    (((ax)-(bx))*((ax)-(bx)) + ((ay)-(by))*((ay)-(by)))
 
#define WRAP_COL(c) (((c) % MAP_COLS + MAP_COLS) % MAP_COLS)
 
#endif /* BASE_H */