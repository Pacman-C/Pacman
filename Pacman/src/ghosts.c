#include "../include/ghosts.h"
#include "../include/map.h"
#include "../include/game.h"
#include "../include/base.h"

#include <stdlib.h>

#define PEN_EXIT_X 14
#define PEN_EXIT_Y 11


static const Uint32 SCATTER_CHASE_TIMERS[] = {
    7000, 
    20000,
    7000, 
    20000,
    5000, 
    20000,
    5000, 
    0     
};

#define SCATTER_CHASE_COUNT 8

static const int PEN_PELLET_TO_GO[GHOST_COUNT] = {
    0,   // BLINKY
    0,   // PINKY
    30,  // INKY
    60   // CLYDE
};

// méthode utlitaire pour avoir la direction opposée (pour éviter de faire demi-tour car c interdit)
    Direction opposite(Direction dir)
{
    if (dir == DIR_UP)    return DIR_DOWN;
    if (dir == DIR_DOWN)  return DIR_UP;
    if (dir == DIR_LEFT)  return DIR_RIGHT;
    if (dir == DIR_RIGHT) return DIR_LEFT;
    return DIR_NONE;
}

// calculer la case cible pour autoriser les fantomes à se diriger vers elle
static int ghost_can_move(Map *map, int x, int y, Direction dir, GhostMode mode)
{
    int nx = WRAP_COL(x + DX[dir]);
    int ny = y + DY[dir];
    char tile = get_tile(map, nx, ny);
    if (tile == TILE_WALL)
    {
        return 0;
    }

    if (mode != GHOST_DEAD && tile == TILE_DOOR)
    {
        return 0;
    }

    return 1;
}

// choisir la direction qui minimise la distance au point cible, IA 
static Direction ghost_choose_dir(Map *map, Ghost *g, int target_x, int target_y)
{
    Direction best_dir = DIR_NONE;
    int best_dist = 999999;
    Direction dirs[4] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};

    for (int i = 0; i < 4; i++) // on test les 4 directions possibles 
    {
        Direction d = dirs[i];
        if (d == opposite(g->entity.dir))
        {
            continue;
        }
        if (!ghost_can_move(map, g->entity.x, g->entity.y, d, g->mode))
        {
            continue;
        }

        // la position suivante si on prend cette direction
        int nx = g->entity.x + DX[d];
        int ny = g->entity.y + DY[d];

        // on calcule la distance au point cible
        int dist = DIST_SQ(nx, ny, target_x, target_y);
        if (dist < best_dist)
        {
            // on garde cette direction comme meilleure si c'est le cas, à la fin on aura la meilleur direction 
            best_dist = dist;
            best_dir  = d;
        }
    }
    return best_dir;
}

static void ghost_move(Ghost *g, Map *map, float delta)
{
    Entity *e = &g->entity;
    char tile = get_tile(map, e->x, e->y);

    // ajuster la vitesse selon le type de tuile et le mode du fantome
    if (tile == TILE_TUNNEL)
    {
        e->speed = SPEED_GHOST_TUNNEL;
    }
    else if (g->mode == GHOST_FRIGHTENED)
    {
        e->speed = SPEED_GHOST_FRIGHT;
    }
    else if (g->mode == GHOST_DEAD)
    {
        e->speed = SPEED_GHOST_DEAD;
    }
    else
    {
        e->speed = SPEED_GHOST;
    }

    float move = e->speed * TILE_SIZE * delta;

    // calculer la nouvelle position en pixels
    float new_px = e->px + DX[e->dir] * move;
    float new_py = e->py + DY[e->dir] * move;

    // calculer la nouvelle position en tuiles
    int new_tx = (int)(new_px / TILE_SIZE);
    int new_ty = (int)(new_py / TILE_SIZE);

    // vérifier les collisions avec les murs
    char next_tile = get_tile(map, new_tx, new_ty);
    if (next_tile == TILE_WALL || (next_tile == TILE_DOOR && g->mode != GHOST_DEAD))
    {
        e->px = e->x * TILE_SIZE;
        e->py = e->y * TILE_SIZE;
        return;
    }

    // mettre à jour la position si pas de collision
    e->px = new_px;
    e->py = new_py;
    e->x  = new_tx;
    e->y  = new_ty;

    if (tile == TILE_TUNNEL)
    {
        if (e->x == 0)
            e->x = MAP_COLS - 1;
        else
            e->x = 0;
        e->px = e->x * TILE_SIZE;
    }
}

static void ghost_leave_pen(Ghost *g)
{
    // voir is le fantome est encore dans la maision
    if (g->entity.y > PEN_EXIT_Y)
    {
        // le faire monter progressivement pour une animation plus fluide
        g->entity.py -= g->entity.speed * TILE_SIZE * 0.016f;
        g->entity.y   = (int)(g->entity.py / TILE_SIZE);
    }
    else
    {
        // il est sorti, on le place à la position de sortie et on change son mode pour qu'il puisse commencer à se déplacer
        g->entity.y   = PEN_EXIT_Y;
        g->entity.py  = PEN_EXIT_Y * TILE_SIZE;
        g->mode       = GHOST_SCATTER;
        g->entity.dir = DIR_LEFT;
    }
}

static void ghost_update_pen(Ghost *g, Map *map)
{
    Uint32 now = SDL_GetTicks();

    if (g->mode_timer != 0 && now - g->mode_timer < 3000)
    {
        return;
    }

    int eaten = map->total_pellets - map->pellet_count;
    if (eaten >= PEN_PELLET_TO_GO[g->id])
    {
        g->mode = GHOST_LEAVING;
    }
}

static void check_collision(Ghost *g, Player *p, Game *game)
{
    float dx = g->entity.px - p->entity.px;
    float dy = g->entity.py - p->entity.py;
    float dist_sq = dx * dx + dy * dy;

    if (dist_sq > 12.0f * 12.0f)
    {
        return;
    }

    if (g->mode == GHOST_FRIGHTENED)
    {
        game->ghosts_eaten_combo++;
        int pts = PTS_GHOST_BASE * (1 << (game->ghosts_eaten_combo - 1));
        p->score += pts;
        g->mode = GHOST_DEAD;
    }
    else if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE)
    {
        game->state = STATE_PACMAN_DEAD;
        game->death_reset_done = 0;
    }
}


void ghost_init(Ghost ghosts[GHOST_COUNT])
{
    ghosts[BLINKY].entity.x     = 14;
    ghosts[BLINKY].entity.y     = 11;
    ghosts[BLINKY].entity.px    = 14 * TILE_SIZE;
    ghosts[BLINKY].entity.py    = 11 * TILE_SIZE;
    ghosts[BLINKY].entity.dir   = DIR_LEFT;
    ghosts[BLINKY].entity.speed = SPEED_GHOST;
    ghosts[BLINKY].id           = BLINKY;
    ghosts[BLINKY].mode         = GHOST_SCATTER;
    ghosts[BLINKY].scatter_x    = 25;
    ghosts[BLINKY].scatter_y    = 0;
    ghosts[BLINKY].mode_timer   = 0;
    ghosts[BLINKY].is_alive     = 1;

    ghosts[PINKY].entity.x     = 14;
    ghosts[PINKY].entity.y     = 14;
    ghosts[PINKY].entity.px    = 14 * TILE_SIZE;
    ghosts[PINKY].entity.py    = 14 * TILE_SIZE;
    ghosts[PINKY].entity.dir   = DIR_DOWN;
    ghosts[PINKY].entity.speed = SPEED_GHOST;
    ghosts[PINKY].id           = PINKY;
    ghosts[PINKY].mode         = GHOST_PEN;
    ghosts[PINKY].scatter_x    = 2;
    ghosts[PINKY].scatter_y    = 0;
    ghosts[PINKY].mode_timer   = 0;
    ghosts[PINKY].is_alive     = 1;

    ghosts[INKY].entity.x     = 12;
    ghosts[INKY].entity.y     = 14;
    ghosts[INKY].entity.px    = 12 * TILE_SIZE;
    ghosts[INKY].entity.py    = 14 * TILE_SIZE;
    ghosts[INKY].entity.dir   = DIR_UP;
    ghosts[INKY].entity.speed = SPEED_GHOST;
    ghosts[INKY].id           = INKY;
    ghosts[INKY].mode         = GHOST_PEN;
    ghosts[INKY].scatter_x    = 27;
    ghosts[INKY].scatter_y    = 30;
    ghosts[INKY].mode_timer   = SDL_GetTicks();
    ghosts[INKY].is_alive     = 1;

    ghosts[CLYDE].entity.x     = 16;
    ghosts[CLYDE].entity.y     = 14;
    ghosts[CLYDE].entity.px    = 16 * TILE_SIZE;
    ghosts[CLYDE].entity.py    = 14 * TILE_SIZE;
    ghosts[CLYDE].entity.dir   = DIR_UP;
    ghosts[CLYDE].entity.speed = SPEED_GHOST;
    ghosts[CLYDE].id           = CLYDE;
    ghosts[CLYDE].mode         = GHOST_PEN;
    ghosts[CLYDE].scatter_x    = 0;
    ghosts[CLYDE].scatter_y    = 30;
    ghosts[CLYDE].mode_timer   = SDL_GetTicks();
    ghosts[CLYDE].is_alive     = 1;
}

static void ghost_compute_target(Ghost *g, Ghost ghosts[GHOST_COUNT], Player *p, int *tx, int *ty)
{
    if (g->mode == GHOST_SCATTER)
    {
        *tx = g->scatter_x;
        *ty = g->scatter_y;
        return;
    }
    if (g->mode == GHOST_DEAD)
    {
        *tx = PEN_EXIT_X;
        *ty = PEN_EXIT_Y;
        return;
    }
    if (g->mode == GHOST_FRIGHTENED)
    {
        *tx = rand() % MAP_COLS;
        *ty = rand() % MAP_ROWS;
        return;
    }
    if (g->id == BLINKY)
    {
        *tx = p->entity.x;
        *ty = p->entity.y;
    }
    else if (g->id == PINKY)
    {
        *tx = p->entity.x + DX[p->entity.dir] * 4;
        *ty = p->entity.y + DY[p->entity.dir] * 4;
        if (p->entity.dir == DIR_UP)
            *tx -= 4;
    }
    else if (g->id == INKY)
    {
        int pivot_x = p->entity.x + DX[p->entity.dir] * 2;
        int pivot_y = p->entity.y + DY[p->entity.dir] * 2;
        int blinky_x = ghosts[BLINKY].entity.x;
        int blinky_y = ghosts[BLINKY].entity.y;
        *tx = pivot_x + (pivot_x - blinky_x);
        *ty = pivot_y + (pivot_y - blinky_y);
    }
    else if (g->id == CLYDE)
    {
        int dist = DIST_SQ(g->entity.x, g->entity.y, p->entity.x, p->entity.y);
        if (dist > CLYDE_THRESHOLD * CLYDE_THRESHOLD)
        {
            *tx = p->entity.x;
            *ty = p->entity.y;
        }
        else
        {
            *tx = g->scatter_x;
            *ty = g->scatter_y;
        }
    }
}


static void update_scatter_chase(Game *game)
{
    if (game->scatter_chase_index >= SCATTER_CHASE_COUNT - 1)
        return;

    Uint32 now = SDL_GetTicks();
    Uint32 duration = SCATTER_CHASE_TIMERS[game->scatter_chase_index];

    // si le timer pour le mode actuel est écoulé, alterner les modes des fantomes entre scatter et chase
    if (now - game->scatter_chase_timer >= duration)
    {
        game->scatter_chase_index++;
        game->scatter_chase_timer = now;

        GhostMode new_mode;

        // alterner entre scatter et chase selon le timer défini dans SCATTER_CHASE_TIMERS
        if (game->scatter_chase_index % 2 == 0)
            new_mode = GHOST_SCATTER;
        else
            new_mode = GHOST_CHASE;

        for (int i = 0; i < GHOST_COUNT; i++)
        {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE)
            {
                g->mode = new_mode;
                g->entity.dir = opposite(g->entity.dir);
            }
        }
    }
}


void ghost_update(Ghost ghosts[GHOST_COUNT], Map *map, Player *p, float delta, Game *game)
{
    update_scatter_chase(game);
    
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        Ghost *g = &ghosts[i];

        if (g->mode == GHOST_PEN)
        {
            ghost_update_pen(g, map);
            continue;
        }
        if (g->mode == GHOST_LEAVING)
        {
            ghost_leave_pen(g);
            continue;
        }

        int target_x, target_y;
        ghost_compute_target(g, ghosts, p, &target_x, &target_y);

        Direction best = ghost_choose_dir(map, g, target_x, target_y);
        if (best != DIR_NONE)
        {
            g->entity.dir = best;
        }
        ghost_move(g, map, delta);

        if (g->mode == GHOST_DEAD && g->entity.x == PEN_EXIT_X && g->entity.y == PEN_EXIT_Y)
        {
            g->mode    = GHOST_PEN;
            g->entity.x  = 14;
            g->entity.y  = 14;
            g->entity.px = 14 * TILE_SIZE;
            g->entity.py = 14 * TILE_SIZE;
            g->mode_timer = SDL_GetTicks();
        }

        check_collision(g, p, game);
    }
}