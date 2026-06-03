#include "../include/ghosts.h"
#include "../include/map.h"
#include "../include/game.h"
#include "../include/base.h"

#include <stdlib.h>
#include <math.h>

#define PEN_EXIT_X 14
#define PEN_EXIT_Y 11

static const Uint32 SCATTER_CHASE_TIMERS[] = {
    7000, 20000, 7000, 20000, 5000, 20000, 5000, 0
};
#define SCATTER_CHASE_COUNT 8

static const int PEN_PELLET_TO_GO[GHOST_COUNT] = { 0, 0, 30, 60 };

Direction opposite(Direction dir)
{
    switch (dir) {
        case DIR_UP:    return DIR_DOWN;
        case DIR_DOWN:  return DIR_UP;
        case DIR_LEFT:  return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        default:        return DIR_NONE;
    }
}

static float apply_level_speed(float base_speed, int level)
{
    if (level <= 1) return base_speed;
    return base_speed + (level - 1) * 0.15f;
}

static int ghost_can_move(Map *map, int x, int y, Direction dir, GhostMode mode)
{
    int nx = WRAP_COL(x + DX[dir]);
    int ny = y + DY[dir];
    char tile = get_tile(map, nx, ny);
    return tile != TILE_WALL && !(tile == TILE_DOOR && mode != GHOST_DEAD);
}

static Direction ghost_choose_dir(Map *map, Ghost *g, int tx, int ty)
{
    Direction dirs[4] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
    Direction best = DIR_NONE;
    int best_dist  = 999999;

    for (int i = 0; i < 4; i++) {
        Direction d = dirs[i];
        if (d == opposite(g->entity.dir)) continue;
        if (!ghost_can_move(map, g->entity.x, g->entity.y, d, g->mode)) continue;

        int nx = g->entity.x + DX[d];
        int ny = g->entity.y + DY[d];
        int dist = DIST_SQ(nx, ny, tx, ty);
        if (dist < best_dist) { best_dist = dist; best = d; }
    }
    return best;
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

static void ghost_move(Ghost *g, Map *map, float delta, int level)
{
    Entity *e  = &g->entity;
    e->speed   = ghost_speed(g, map, level);
    float move = e->speed * TILE_SIZE * delta;

    int cx = (int)((e->px + DX[e->dir] * move) / TILE_SIZE);
    int cy = (int)((e->py + DY[e->dir] * move) / TILE_SIZE);
    char next = get_tile(map, cx, cy);

    if (next == TILE_WALL || (next == TILE_DOOR && g->mode != GHOST_DEAD)) {
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

static void ghost_leave_pen(Ghost *g, float delta)
{
    float target_px = PEN_EXIT_X * TILE_SIZE;
    g->entity.speed = apply_level_speed(SPEED_GHOST, level);
    float move = g->entity.speed * TILE_SIZE * delta;

    if (g->mode == GHOST_LEAVING) {
        ghost_leave_pen(g, delta, game->level);  
        continue;
    }

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
        g->mode       = GHOST_SCATTER;
        g->entity.dir = DIR_LEFT;
    }
}

static void ghost_return_to_pen(Ghost *g)
{
    g->entity.x  = 14;
    g->entity.y  = 14;
    g->entity.px = 14 * TILE_SIZE;
    g->entity.py = 14 * TILE_SIZE;
    g->mode      = GHOST_LEAVING;
}

static void ghost_compute_target(Ghost *g, Ghost ghosts[GHOST_COUNT],
                                  Player *p, int *tx, int *ty)
{
    switch (g->mode) {
        case GHOST_SCATTER:
            *tx = g->scatter_x; *ty = g->scatter_y; return;
        case GHOST_DEAD:
            *tx = PEN_EXIT_X;   *ty = PEN_EXIT_Y;   return;
        case GHOST_FRIGHTENED:
            *tx = rand() % MAP_COLS; *ty = rand() % MAP_ROWS; return;
        default: break;
    }

    switch (g->id) {
        case BLINKY:
            *tx = p->entity.x;
            *ty = p->entity.y;
            break;
        case PINKY:
            *tx = p->entity.x + DX[p->entity.dir] * 4;
            *ty = p->entity.y + DY[p->entity.dir] * 4;
            if (p->entity.dir == DIR_UP) *tx -= 4;
            break;
        case INKY: {
            int pvx = p->entity.x + DX[p->entity.dir] * 2;
            int pvy = p->entity.y + DY[p->entity.dir] * 2;
            *tx = pvx + (pvx - ghosts[BLINKY].entity.x);
            *ty = pvy + (pvy - ghosts[BLINKY].entity.y);
            break;
        }
        case CLYDE:
            if (DIST_SQ(g->entity.x, g->entity.y, p->entity.x, p->entity.y)
                    > CLYDE_THRESHOLD * CLYDE_THRESHOLD)
                { *tx = p->entity.x; *ty = p->entity.y; }
            else
                { *tx = g->scatter_x; *ty = g->scatter_y; }
            break;
        default: break;
    }
}

static void check_collision(Ghost *g, Player *p, Game *game)
{
    float dx = (g->entity.px + TILE_SIZE / 2) - p->entity.px;
    float dy = (g->entity.py + TILE_SIZE / 2) - p->entity.py;
    float seuil = TILE_SIZE * 3 / 4;
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

static void update_scatter_chase(Game *game)
{
    if (game->scatter_chase_index >= SCATTER_CHASE_COUNT - 1) return;

    Uint32 now = SDL_GetTicks();
    if (now - game->scatter_chase_timer < SCATTER_CHASE_TIMERS[game->scatter_chase_index])
        return;

    game->scatter_chase_index++;
    game->scatter_chase_timer = now;
    GhostMode new_mode = (game->scatter_chase_index % 2 == 0) ? GHOST_SCATTER : GHOST_CHASE;

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE) {
            g->mode       = new_mode;
            g->entity.dir = opposite(g->entity.dir);
        }
    }
}

void ghost_update(Ghost ghosts[GHOST_COUNT], Map *map, Player *p,
                  float delta, Game *game)
{
    update_scatter_chase(game);

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &ghosts[i];

        if (g->mode == GHOST_PEN) {
            int eaten = map->total_pellets - map->pellet_count;
            if (eaten >= PEN_PELLET_TO_GO[g->id])
                g->mode = GHOST_LEAVING;
            continue;
        }
        if (g->mode == GHOST_LEAVING) {
            ghost_leave_pen(g, delta);
            continue;
        }

        float dpx = g->entity.px - g->entity.x * TILE_SIZE;
        float dpy = g->entity.py - g->entity.y * TILE_SIZE;
        if (dpx * dpx + dpy * dpy < 4.0f) {
            int tx, ty;
            ghost_compute_target(g, ghosts, p, &tx, &ty);
            g->target_x = tx;
            g->target_y = ty;
            Direction best = ghost_choose_dir(map, g, tx, ty);
            if (best != DIR_NONE) g->entity.dir = best;
        }

        check_collision(g, p, game);

        ghost_move(g, map, delta, game->level);

        if (g->mode == GHOST_DEAD &&
            g->entity.x == PEN_EXIT_X && g->entity.y == PEN_EXIT_Y)
            ghost_return_to_pen(g);

        check_collision(g, p, game);
    }
}