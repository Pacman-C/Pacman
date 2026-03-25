#ifndef GAME_H
#define GAME_H

#include "base.h"

void game_init(Game *game);
void game_update(Game *game, float delta);
void handle_input(Game *game);

#endif /* GAME_H */