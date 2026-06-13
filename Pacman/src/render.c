#include "../include/render.h"
#include "../include/base.h"
#include <stdio.h>
#include <stdlib.h>

static SDL_Window   *g_win    = NULL;
static SDL_Renderer *g_ren    = NULL;
static SDL_Texture  *g_sprite = NULL;

static int g_debug = 0;

SDL_Renderer* render_get_renderer(void)
{
    return g_ren;
}

SDL_Texture* render_get_sprite(void)
{
    return g_sprite;
}

void render_toggle_debug(void)
{
    g_debug = !g_debug;
    render_debug_set_enabled(g_debug);
}

void render_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    g_win = SDL_CreateWindow(
        "Pac-Man",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, 0
    );
    if (!g_win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); exit(1); }

    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
    if (!g_ren) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); exit(1); }

    SDL_RenderSetLogicalSize(g_ren, WINDOW_W, WINDOW_H);

    SDL_Surface *surf = SDL_LoadBMP("assets/sprites.bmp");
    if (!surf) { fprintf(stderr, "SDL_LoadBMP: %s\n", SDL_GetError()); exit(1); }
    g_sprite = SDL_CreateTextureFromSurface(g_ren, surf);
    SDL_FreeSurface(surf);
    if (!g_sprite) { fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", SDL_GetError()); exit(1); }

    render_map_init();
    render_entities_init();
    render_debug_init();
}

void render_frame(const Game *game)
{
    if (game->state == STATE_TITLE) {
        render_screen_title();
        return;
    }

    if (game->state == STATE_LEVEL_TRANSITION) {
        render_screen_level_transition(game);
        return;
    }

    if (game->state == STATE_WIN) {
        render_screen_victory(game->level);
        return;
    }

    if (game->state == STATE_GAMEOVER) {
        render_screen_game_over(game->player.score);
        return;
    }

    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);

    Uint32 t     = SDL_GetTicks();
    int    flash = (game->state == STATE_WIN) ? (t / 200) % 2 : 0;

    render_map_draw(game, t, flash);
    render_entities_draw(game, t);
    
    if (g_debug) {
        render_debug_draw_targets(game);
        render_debug_draw_ghost_modes(game);
        render_debug_draw_phase(game, t);
    }

    render_debug_draw_lives(game);
    render_debug_draw_score(game);
    render_debug_draw_ghost_score(game);

    SDL_RenderPresent(g_ren);
}

void render_quit(void)
{
    if (g_sprite) SDL_DestroyTexture(g_sprite);
    if (g_ren)    SDL_DestroyRenderer(g_ren);
    if (g_win)    SDL_DestroyWindow(g_win);
    SDL_Quit();
}