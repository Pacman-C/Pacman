#include "../include/pacman.h"
#include "../include/map.h"

static const int DX[] = { 0,  0, -1,  1};
static const int DY[] = {-1,  1,  0,  0};

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

void pacman_update(Player *p, Map *map, float delta)
{
    Entity *e = &p->entity;

    if (e->next_dir != DIR_NONE)
    {
        if (can_move(map, e->x, e->y, e->next_dir))
        {
            e->dir = e->next_dir;
            e->next_dir = DIR_NONE;
        }
    }

    if (e->dir == DIR_NONE)
    {
        return;
    }

    if (!can_move(map, e->x, e->y, e->dir))
    {
        return;
    }

    float move = e->speed * TILE_SIZE * delta;

    e->px += (int)(DX[e->dir] * move);
    e->py += (int)(DY[e->dir] * move);

    e->x = e->px / TILE_SIZE;
    e->y = e->py / TILE_SIZE;

    char tile = get_tile(map, e->x, e->y);

    if (tile == TILE_TUNNEL)
    {
        if (e->x == 0)
        {
            e->x = MAP_COLS - 1;
        }
        else
        {
            e->x = 0;
        }
        e->px = e->x * TILE_SIZE;
    }

    if (tile == TILE_PELLET || tile == TILE_POWER_PELLET)
    {
        e->speed = SPEED_PACMAN_EATING;
    }
    else
    {
        e->speed = SPEED_PACMAN;
    }
}

void pacman_set_dir(Player *p, Direction dir)
{
    p->entity.next_dir = dir;
}