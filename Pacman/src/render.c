#include "../include/render.h"
#include <stdio.h>
#include <stdlib.h>

/* Contexte global au fichier — inaccessible depuis l'extérieur */
static SDL_Window   *g_win = NULL;
static SDL_Renderer *g_ren = NULL;
static SDL_Texture  *g_sprite = NULL; 

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
    SDL_RenderSetLogicalSize(g_ren, WINDOW_W, WINDOW_H);
    if (!g_ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_RenderSetLogicalSize(g_ren, WINDOW_W, WINDOW_H);
       SDL_Surface *surf = SDL_LoadBMP("assets/sprites.bmp");
    if (!surf) {
        fprintf(stderr, "SDL_LoadBMP: %s\n", SDL_GetError());
        exit(1);
    }
    g_sprite = SDL_CreateTextureFromSurface(g_ren, surf);
    SDL_FreeSurface(surf);
    if (!g_sprite) {
        fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(1);
    }
}

void render_frame(const Game *game)
{
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);

    Uint32 t = SDL_GetTicks();
    int flash = (game->state == STATE_WIN) ? (t / 200) % 2 : 0;

    /* ================= MAP ================= */
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {

            char tile = game->map.grid[r][c];
            SDL_Rect dst = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            if (tile == TILE_WALL) {
                if (flash) {
                    SDL_SetRenderDrawColor(g_ren, 255, 255, 255, 255);
                    SDL_RenderFillRect(g_ren, &dst);
                } else {
                    SDL_SetRenderDrawColor(g_ren, 0, 0, 139, 255);
                    SDL_RenderFillRect(g_ren, &dst);
                }
            }
            else if (tile == TILE_PELLET) {
                int cx = c * TILE_SIZE + TILE_SIZE / 2;
                int cy = r * TILE_SIZE + TILE_SIZE / 2;

                SDL_SetRenderDrawColor(g_ren, 255, 184, 151, 255);
                SDL_RenderDrawPoint(g_ren, cx, cy);
            }
            else if (tile == TILE_POWER_PELLET) {
                if ((t / 300) % 2) {
                    int cx = c * TILE_SIZE + TILE_SIZE / 2;
                    int cy = r * TILE_SIZE + TILE_SIZE / 2;

                    SDL_SetRenderDrawColor(g_ren, 255, 184, 151, 255);
                    SDL_Rect p = { cx - 2, cy - 2, 4, 4 };
                    SDL_RenderFillRect(g_ren, &p);
                }
            }
        }
    }

    /* ================= PACMAN ================= */
    int cx = (int)game->player.entity.px;
    int cy = (int)game->player.entity.py;
    int radius = TILE_SIZE / 2 - 1;

    SDL_SetRenderDrawColor(g_ren, 255, 255, 0, 255);

    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                SDL_RenderDrawPoint(g_ren, cx + dx, cy + dy);
            }
        }
    }

    /* ================= GHOSTS ================= */
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        Ghost *g = &game->ghosts[i];

        SDL_Rect dst = {
            (int)g->entity.px,
            (int)g->entity.py,
            TILE_SIZE,
            TILE_SIZE
        };

        SDL_Rect src = {0, 0, TILE_SIZE, TILE_SIZE};

        /* NORMAL + SCATTER + CHASE + PEN + LEAVING */
        if (g->mode == GHOST_SCATTER ||
            g->mode == GHOST_CHASE ||
            g->mode == GHOST_LEAVING ||
            g->mode == GHOST_PEN)
        {
            src.x = g->id * TILE_SIZE;
            src.y = 0;
        }

        /* FRIGHTENED */
        else if (g->mode == GHOST_FRIGHTENED)
        {
            int blink = (t / 200) % 2;
            src.y = TILE_SIZE;
            src.x = (blink == 0) ? 0 : TILE_SIZE;
        }

        /* DEAD */
        else if (g->mode == GHOST_DEAD)
        {
            src.y = 2 * TILE_SIZE;

            if (g->entity.dir == DIR_LEFT)      src.x = 0;
            else if (g->entity.dir == DIR_RIGHT) src.x = TILE_SIZE;
            else if (g->entity.dir == DIR_UP)    src.x = 2 * TILE_SIZE;
            else if (g->entity.dir == DIR_DOWN)  src.x = 3 * TILE_SIZE;
            else src.x = 0;
        }

        SDL_RenderCopy(g_ren, g_sprite, &src, &dst);
    }

    SDL_RenderPresent(g_ren);
}


void render_quit(void)
{
    if (g_sprite) SDL_DestroyTexture(g_sprite);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}