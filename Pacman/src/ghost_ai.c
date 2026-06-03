#include "../include/ghosts.h"
#include "../include/map.h"
#include "../include/game.h"
#include "../include/base.h"
#include <stdlib.h>

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

static Direction ghost_choose_dir(Map *map, Ghost *g, int tx, int ty)
{
    Direction dirs[4] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
    Direction best = DIR_NONE;
    int best_dist  = 999999;

    for (int i = 0; i < 4; i++) {
        Direction d = dirs[i];
        if (d == opposite(g->entity.dir)) continue;
        
        int nx = WRAP_COL(g->entity.x + DX[d]);
        int ny = g->entity.y + DY[d];
        char tile = get_tile(map, nx, ny);
        
        if (tile == TILE_WALL) continue;
        if (tile == TILE_DOOR && g->mode != GHOST_DEAD) continue;

        int dist = DIST_SQ(nx, ny, tx, ty);
        if (dist < best_dist) { best_dist = dist; best = d; }
    }
    return best;
}

static void ghost_compute_target(Ghost *g, Ghost ghosts[GHOST_COUNT],
                                  Player *p, int *tx, int *ty)
{
    switch (g->mode) {
        case GHOST_SCATTER:
            *tx = g->scatter_x; *ty = g->scatter_y; return;
        case GHOST_DEAD:
            *tx = 14; *ty = 11; return;
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

void ghost_choose_next_move(Ghost *g, Ghost ghosts[GHOST_COUNT], Map *map, Player *p)
{
    int tx, ty;
    ghost_compute_target(g, ghosts, p, &tx, &ty);
    g->target_x = tx;
    g->target_y = ty;
    
    Direction best = ghost_choose_dir(map, g, tx, ty);
    if (best != DIR_NONE) g->entity.dir = best;
}

static void update_scatter_chase(Game *game)
{
    static const Uint32 SCATTER_CHASE_TIMERS[] = {
        7000, 20000, 7000, 20000, 5000, 20000, 5000, 0
    };
    
    if (game->scatter_chase_index >= 7) return;

    Uint32 now = SDL_GetTicks();
    if (now - game->scatter_chase_timer < SCATTER_CHASE_TIMERS[game->scatter_chase_index])
        return;

    game->scatter_chase_index++;
    game->scatter_chase_timer = now;
    GhostMode new_mode = (game->scatter_chase_index % 2 == 0) ? GHOST_SCATTER : GHOST_CHASE;

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE) {
            g->mode_before_fright = g->mode;  // ← Sauvegarder mode avant changement
            g->mode       = new_mode;
            g->entity.dir = opposite(g->entity.dir);
        }
    }
}

void ghost_update_modes(Ghost ghosts[GHOST_COUNT], Map *map, Game *game)
{
    update_scatter_chase(game);

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &ghosts[i];

        if (g->mode == GHOST_PEN) {
            static const int PEN_PELLET_TO_GO[] = { 0, 0, 30, 60 };
            int eaten = map->total_pellets - map->pellet_count;
            
            /* Délai de 1000ms après mort SEULEMENT si mode_timer > 0 */
            int should_wait_delay = (g->mode_timer > 0);
            Uint32 time_in_pen = should_wait_delay ? (SDL_GetTicks() - g->mode_timer) : 0;
            
            if (eaten >= PEN_PELLET_TO_GO[g->id] && (!should_wait_delay || time_in_pen >= 1000)) {
                g->mode = GHOST_LEAVING;
            }
        }
    }
}