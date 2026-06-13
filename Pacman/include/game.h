#ifndef GAME_H
#define GAME_H

#include "base.h"

void game_init(Game *game);
void game_update(Game *game, float delta);
void handle_input(Game *game);
void game_start_next_level(Game *game);

#endif /* GAME_H */