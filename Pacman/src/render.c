#include "../include/render.h"
#include <stdio.h>
#include <stdlib.h>

/* Contexte global au fichier — inaccessible depuis l'extérieur */
static SDL_Window   *g_win = NULL;
static SDL_Renderer *g_ren = NULL;

void render_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    g_win = SDL_CreateWindow(
        "Pac-Man",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        0
    );
    if (!g_win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        exit(1);
    }

    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
    if (!g_ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(1);
    }
}

void render_frame(const Game *game)
{
    (void)game; /* sera utilisé dès la séance 2 */

    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);

    /* TODO séance 2 : dessiner la grille, Pac-Man, les fantômes */

    SDL_RenderPresent(g_ren);
}

void render_quit(void)
{
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}