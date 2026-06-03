#ifndef GHOSTS_H
#define GHOSTS_H

#include "../include/base.h"

#define PEN_EXIT_X 14
#define PEN_EXIT_Y 11

Direction opposite(Direction dir);
void ghost_init(Ghost ghosts[GHOST_COUNT]);
void ghost_move(Ghost *g, Map *map, float delta, int level);
void ghost_leave_pen(Ghost *g, float delta, int level);
void ghost_return_to_pen(Ghost *g);
void ghost_check_collision(Ghost *g, Player *p, Game *game);
void ghost_choose_next_move(Ghost *g, Ghost ghosts[GHOST_COUNT], Map *map, Player *p);
void ghost_update_modes(Ghost ghosts[GHOST_COUNT], Map *map, Game *game);
void ghost_update(Ghost ghosts[GHOST_COUNT], Map *map, Player *p, float delta, Game *game);

#endif