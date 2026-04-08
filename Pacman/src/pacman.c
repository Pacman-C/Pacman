#include "../include/pacman.h"
#include "../include/map.h"
#include "../include/game.h"

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

    /* Tuile actuelle depuis le centre */
    e->x = (int)((e->px - TILE_SIZE / 2) / TILE_SIZE);
    e->y = (int)((e->py - TILE_SIZE / 2) / TILE_SIZE);

    /* Changement de direction si possible */
    if (e->next_dir != DIR_NONE)
{
    /* Vérifie que Pac-Man est assez centré sur sa case */
    float center_px = e->x * TILE_SIZE + TILE_SIZE / 2;
    float center_py = e->y * TILE_SIZE + TILE_SIZE / 2;
    float dist_x = e->px - center_px;
    float dist_y = e->py - center_py;
    int centered = (dist_x * dist_x + dist_y * dist_y) < 16.0f;

    if (centered && can_move(map, e->x, e->y, e->next_dir))
    {
        e->dir = e->next_dir;
        e->next_dir = DIR_NONE;
    }
}

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

    /* Force centrage sur l'axe perpendiculaire */
    if (e->dir == DIR_LEFT || e->dir == DIR_RIGHT)
        e->py = e->y * TILE_SIZE + TILE_SIZE / 2;
    if (e->dir == DIR_UP || e->dir == DIR_DOWN)
        e->px = e->x * TILE_SIZE + TILE_SIZE / 2;

    /* Recalcule la tuile après déplacement */
    e->x = (int)((e->px - TILE_SIZE / 2) / TILE_SIZE);
    e->y = (int)((e->py - TILE_SIZE / 2) / TILE_SIZE);

    /* Tunnel */
    char tile = get_tile(map, e->x, e->y);
    if (tile == TILE_TUNNEL)
    {
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
        tile = get_tile(map, e->x, e->y);
    }

    /* Vitesse */
    if (tile == TILE_PELLET || tile == TILE_POWER_PELLET)
        e->speed = SPEED_PACMAN_EATING;
    else
        e->speed = SPEED_PACMAN;

    /* Manger */
    if (tile == TILE_PELLET)
    {
        set_tile(map, e->x, e->y, TILE_EMPTY);
        map->pellet_count--;
        p->score += PTS_DOT;
    }
    else if (tile == TILE_POWER_PELLET)
    {
        p->score += PTS_POWER_PELLET;
        set_tile(map, e->x, e->y, TILE_EMPTY);
        p->is_powered = 1;
        p->power_timer = SDL_GetTicks();
        map->pellet_count--;
    }
}


void pacman_set_dir(Player *p, Direction dir)
{
    p->entity.next_dir = dir;
}