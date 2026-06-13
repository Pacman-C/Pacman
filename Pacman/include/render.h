#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "base.h"
#include "render_screens.h"
#include "audio.h"

void render_init(void);
void render_frame(const Game *game);
void render_quit(void);
void render_toggle_debug(void);

SDL_Renderer* render_get_renderer(void);
SDL_Texture* render_get_sprite(void);

void render_map_init(void);
void render_map_draw(const Game *game, Uint32 t, int flash);

void render_entities_init(void);
void render_entities_draw(const Game *game, Uint32 t);

void render_debug_init(void);
void render_debug_set_enabled(int enabled);
void render_debug_draw_targets(const Game *game);
void render_debug_draw_ghost_modes(const Game *game);
void render_debug_draw_phase(const Game *game, Uint32 t);
void render_debug_draw_lives(const Game *game);
void render_debug_draw_score(const Game *game);
void render_debug_draw_ghost_score(const Game *game);

#endif /* RENDER_H */