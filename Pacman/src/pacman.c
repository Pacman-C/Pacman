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

    float new_px = e->px + DX[e->dir] * move;
    float new_py = e->py + DY[e->dir] * move;

    int new_tx = (int)(new_px / TILE_SIZE);
    int new_ty = (int)(new_py / TILE_SIZE);

    // Vérifier la tuile destination avant de bouger
    char next_tile = get_tile(map, new_tx, new_ty);
    if (next_tile == TILE_WALL || next_tile == TILE_DOOR)
    {
        // Snap Pac-Man au centre de sa tuile actuelle
        e->px = e->x * TILE_SIZE;
        e->py = e->y * TILE_SIZE;
        return;
    }

    e->px = new_px;
    e->py = new_py;
    e->x  = new_tx;
    e->y  = new_ty;

    // Tunnel
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

    // Vitesse
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