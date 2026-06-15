#ifndef BASE_H
#define BASE_H

#include <SDL2/SDL.h>
#include "constants.h"
#include "types.h"
#include "entities.h"

/* ── Carte ───────────────────────────────────── */
typedef struct {
    char grid[MAP_ROWS][MAP_COLS];
    int  pellet_count;
    int  total_pellets;
} Map;

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
    int       force_leave;
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
    FruitType fruit_type;

    int       level;
    int       high_score;
    int       ghosts_eaten_combo;
    Uint32    frightened_start;
    Uint32    death_start;
    Uint32    last_tick;

    int       death_reset_done;
    int       scatter_chase_index;
    Uint32    scatter_chase_timer;

    int       fruit_x, fruit_y;
    int       fruit_active;
    Uint32    fruit_timer;
    int       fruit_spawn_count;

    int       ghost_score_visible;
    int       ghost_score_value;
    int       ghost_score_x;
    int       ghost_score_y;
    Uint32    ghost_score_timer;

    int       extra_life_earned;
    Uint32    level_transition_start;
} Game;

#endif /* BASE_H */