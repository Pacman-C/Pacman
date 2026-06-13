#ifndef RENDER_SCREENS_H
#define RENDER_SCREENS_H

#include "base.h"

void render_screen_title(void);
void render_screen_game_over(int final_score);
void render_screen_victory(int level);
void render_screen_level_transition(const Game *game);

#endif // RENDER_SCREENS_H