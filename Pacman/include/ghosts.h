#ifndef GHOST_H
#define GHOST_H

#include "base.h"

void ghost_init(Ghost ghosts[GHOST_COUNT]);
void ghost_update(Ghost ghosts[GHOST_COUNT], Map *map, Player *p, float delta, Game *game);

#endif /* GHOST_H */