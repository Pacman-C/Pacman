#include "../include/pacman.h"
#include "../include/map.h"
#include "../include/game.h"
#include "../include/audio.h"

static int can_move(Map *map, int x, int y, Direction dir)
{
    int nx = WRAP_COL(x + DX[dir]);
    int ny = y + DY[dir];
    char tile = get_tile(map, nx, ny);

    if (tile == TILE_WALL || tile == TILE_DOOR)
    {
        return 0;
    }
    return 1;
}

static void pacman_update_tile_pos(Entity *e)
{
    e->x = (int)((e->px - TILE_SIZE / 2) / TILE_SIZE);
    e->y = (int)((e->py - TILE_SIZE / 2) / TILE_SIZE);
}

static void pacman_try_turn(Entity *e, Map *map)
{
    if (e->next_dir == DIR_NONE)
        return;

    float center_px = e->x * TILE_SIZE + TILE_SIZE / 2;
    float center_py = e->y * TILE_SIZE + TILE_SIZE / 2;
    float dist_x = e->px - center_px;
    float dist_y = e->py - center_py;

    if ((dist_x * dist_x + dist_y * dist_y) < 16.0f
        && can_move(map, e->x, e->y, e->next_dir))
    {
        e->dir = e->next_dir;
        e->next_dir = DIR_NONE;
    }
}

static void pacman_move(Entity *e, Map *map, float delta)
{
    if (e->dir == DIR_NONE)
        return;

    if (!can_move(map, e->x, e->y, e->dir))
    {
        e->px = e->x * TILE_SIZE + TILE_SIZE / 2;
        e->py = e->y * TILE_SIZE + TILE_SIZE / 2;
        return;
    }

    float move = e->speed * TILE_SIZE * delta;
    e->px += DX[e->dir] * move;
    e->py += DY[e->dir] * move;

    if (e->dir == DIR_LEFT || e->dir == DIR_RIGHT)
        e->py = e->y * TILE_SIZE + TILE_SIZE / 2;
    else
        e->px = e->x * TILE_SIZE + TILE_SIZE / 2;
}


static void pacman_handle_tunnel(Entity *e, Map *map)
{
    if (get_tile(map, e->x, e->y) != TILE_TUNNEL)
        return;

    if (e->dir == DIR_LEFT)
    {
        e->x  = MAP_COLS - 2;
        e->px = e->x * TILE_SIZE + TILE_SIZE / 2;
    }
    else if (e->dir == DIR_RIGHT)
    {
        e->x  = 1;
        e->px = e->x * TILE_SIZE + TILE_SIZE / 2;
    }
    e->py = e->y * TILE_SIZE + TILE_SIZE / 2;
}

static void pacman_collect_tile(Player *p, Map *map, Game *game)
{
    Entity *e = &p->entity;
    char tile = get_tile(map, e->x, e->y);
    int ate_pellet = 0;

    if (tile == TILE_PELLET)
    {
        set_tile(map, e->x, e->y, TILE_EMPTY);
        map->pellet_count--;
        p->score += PTS_DOT;

        ate_pellet = 1;
    }
    else if (tile == TILE_POWER_PELLET)
    {
        set_tile(map, e->x, e->y, TILE_EMPTY);
        map->pellet_count--;
        p->score += PTS_POWER_PELLET;
        p->is_powered = 1;
        p->power_timer = SDL_GetTicks();

        audio_play(SOUND_POWER_UP);
        ate_pellet = 1;
    }
    else if (tile == TILE_FRUIT && game->fruit_active)
    {
        set_tile(map, e->x, e->y, TILE_EMPTY);
        game->fruit_active = 0;

        int pts = 0;
        switch (game->fruit_type) {
            case FRUIT_CHERRY:     pts = PTS_CHERRY;      break;
            case FRUIT_STRAWBERRY: pts = PTS_STRAWBERRY;  break;
            case FRUIT_ORANGE:     pts = PTS_ORANGE;      break;
            case FRUIT_APPLE:      pts = PTS_APPLE;       break;
            case FRUIT_MELON:      pts = PTS_MELON;       break;
            case FRUIT_GALAXIAN:   pts = PTS_GALAXIAN;    break;
            case FRUIT_BELL:       pts = PTS_BELL;        break;
            case FRUIT_KEY:        pts = PTS_KEY;         break;
        }
        p->score += pts;

        game->ghost_score_visible = 1;
        game->ghost_score_value   = pts;
        game->ghost_score_x       = e->x;
        game->ghost_score_y       = e->y;
        game->ghost_score_timer   = SDL_GetTicks();
    }

    if (ate_pellet)
    {
        audio_play(SOUND_CHOMP);

        e->speed = SPEED_PACMAN_EATING + (game->level - 1) * 0.1f;
    }
    else
    {
        e->speed = SPEED_PACMAN + (game->level - 1) * 0.1f;
    }
}


void pacman_update(Player *p, Map *map, float delta, Game *game)
{
    Entity *e = &p->entity;

    pacman_update_tile_pos(e);
    pacman_try_turn(e, map);
    pacman_move(e, map, delta);
    pacman_update_tile_pos(e);     
    pacman_handle_tunnel(e, map);
    pacman_collect_tile(p, map, game);
}


void pacman_set_dir(Player *p, Direction dir)
{
    p->entity.next_dir = dir;
}