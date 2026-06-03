#include "../include/ghosts.h"
#include "../include/map.h"
#include "../include/game.h"
#include "../include/base.h"
#include <math.h>

static float apply_level_speed(float base_speed, int level)
{
    if (level <= 1) return base_speed;
    return base_speed + (level - 1) * 0.15f;
}

static float ghost_speed(Ghost *g, Map *map, int level)
{
    char tile = get_tile(map, g->entity.x, g->entity.y);
    if (tile == TILE_TUNNEL)         return SPEED_GHOST_TUNNEL;
    if (g->mode == GHOST_FRIGHTENED) return apply_level_speed(SPEED_GHOST_FRIGHT, level);
    if (g->mode == GHOST_DEAD)       return SPEED_GHOST_DEAD;

    float spd = apply_level_speed(SPEED_GHOST, level);
    if (g->id == BLINKY && g->mode == GHOST_CHASE) {
        if (map->pellet_count <= 10)      spd *= 1.4f;
        else if (map->pellet_count <= 20) spd *= 1.2f;
    }
    return spd;
}

void ghost_move(Ghost *g, Map *map, float delta, int level)
{
    Entity *e  = &g->entity;
    e->speed   = ghost_speed(g, map, level);
    float move = e->speed * TILE_SIZE * delta;

    int cx = (int)((e->px + DX[e->dir] * move) / TILE_SIZE);
    int cy = (int)((e->py + DY[e->dir] * move) / TILE_SIZE);
    char next = get_tile(map, cx, cy);

    if (next == TILE_WALL || (next == TILE_DOOR && g->mode != GHOST_DEAD && g->mode != GHOST_LEAVING)) {
        e->px = e->x * TILE_SIZE;
        e->py = e->y * TILE_SIZE;
        return;
    }

    e->px += DX[e->dir] * move;
    e->py += DY[e->dir] * move;
    e->x   = (int)(e->px / TILE_SIZE);
    e->y   = (int)(e->py / TILE_SIZE);

    if (e->x <= 0 && e->dir == DIR_LEFT)
        { e->x = MAP_COLS - 1; e->px = e->x * TILE_SIZE; }
    else if (e->x >= MAP_COLS - 1 && e->dir == DIR_RIGHT)
        { e->x = 0; e->px = 0; }
}

void ghost_leave_pen(Ghost *g, float delta, int level)
{
    float target_px = PEN_EXIT_X * TILE_SIZE;
    g->entity.speed = apply_level_speed(SPEED_GHOST, level);
    float move = g->entity.speed * TILE_SIZE * delta;

    if (fabsf(g->entity.px - target_px) > 0.5f) {
        g->entity.px += (g->entity.px < target_px) ? move : -move;
        g->entity.x   = (int)(g->entity.px / TILE_SIZE);
        return;
    }
    if (g->entity.y > PEN_EXIT_Y) {
        g->entity.py -= move;
        g->entity.y   = (int)(g->entity.py / TILE_SIZE);
    } else {
        g->entity.y   = PEN_EXIT_Y;
        g->entity.py  = PEN_EXIT_Y * TILE_SIZE;
        g->mode       = g->mode_before_fright; 
        g->entity.dir = DIR_LEFT;
    }
}

void ghost_return_to_pen(Ghost *g)
{
    g->entity.x  = 14;
    g->entity.y  = 14;
    g->entity.px = 14 * TILE_SIZE;
    g->entity.py = 14 * TILE_SIZE;
    g->mode      = GHOST_PEN; 
    g->mode_timer = SDL_GetTicks(); 
}

void ghost_check_collision(Ghost *g, Player *p, Game *game)
{
    float dx = (g->entity.px + TILE_SIZE / 2) - p->entity.px;
    float dy = (g->entity.py + TILE_SIZE / 2) - p->entity.py;
    float seuil = TILE_SIZE; 
    if (dx * dx + dy * dy > seuil * seuil) return;

    if (g->mode == GHOST_FRIGHTENED) {
        game->ghosts_eaten_combo++;
        int pts = PTS_GHOST_BASE * (1 << (game->ghosts_eaten_combo - 1));
        p->score += pts;
        game->ghost_score_visible = 1;
        game->ghost_score_value   = pts;
        game->ghost_score_x       = g->entity.x;
        game->ghost_score_y       = g->entity.y;
        game->ghost_score_timer   = SDL_GetTicks();
        g->mode = GHOST_DEAD;
    } else if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE) {
        game->state            = STATE_PACMAN_DEAD;
        game->death_reset_done = 0;
        game->death_start      = SDL_GetTicks();
    }
}

void ghost_init(Ghost ghosts[GHOST_COUNT])
{
    typedef struct { int x, y; GhostMode mode; int sx, sy; Direction dir; } GhostDef;
    static const GhostDef defs[GHOST_COUNT] = {
        { 14, 11, GHOST_SCATTER, 25,  0, DIR_LEFT },
        { 14, 14, GHOST_PEN,      2,  0, DIR_DOWN },
        { 12, 14, GHOST_PEN,     27, 30, DIR_UP   },
        { 16, 14, GHOST_PEN,      0, 30, DIR_UP   },
    };

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g        = &ghosts[i];
        g->entity.x     = defs[i].x;
        g->entity.y     = defs[i].y;
        g->entity.px    = defs[i].x * TILE_SIZE;
        g->entity.py    = defs[i].y * TILE_SIZE;
        g->entity.dir   = defs[i].dir;
        g->entity.speed = SPEED_GHOST;
        g->id           = i;
        g->mode         = defs[i].mode;
        g->scatter_x    = defs[i].sx;
        g->scatter_y    = defs[i].sy;
        g->mode_timer   = 0;
        g->is_alive     = 1;
        g->mode_before_fright = GHOST_SCATTER;
    }
}

void ghost_update(Ghost ghosts[GHOST_COUNT], Map *map, Player *p, float delta, Game *game)
{
    ghost_update_modes(ghosts, map, game);

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &ghosts[i];

        if (g->mode == GHOST_PEN) continue;  
        
        if (g->mode == GHOST_LEAVING) {
            ghost_leave_pen(g, delta, game->level);
            ghost_check_collision(g, p, game);
            continue;
        }

        float dpx = g->entity.px - g->entity.x * TILE_SIZE;
        float dpy = g->entity.py - g->entity.y * TILE_SIZE;
        if (dpx * dpx + dpy * dpy < 4.0f) {
            ghost_choose_next_move(g, ghosts, map, p);
        }

        ghost_check_collision(g, p, game);  // ← AVANT move
        ghost_move(g, map, delta, game->level);
        ghost_check_collision(g, p, game);  // ← APRÈS move

        if (g->mode == GHOST_DEAD &&
            g->entity.x == PEN_EXIT_X && g->entity.y == PEN_EXIT_Y)
            ghost_return_to_pen(g);
    }
}