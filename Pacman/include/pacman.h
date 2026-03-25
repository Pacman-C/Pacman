#ifndef PACMAN_H
#define PACMAN_H

#include "base.h"

void pacman_update(Player *p, Map *map, float delta);
void pacman_set_dir(Player *p, Direction dir);

#endif