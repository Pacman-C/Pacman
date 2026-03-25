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
    if (!g_ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(1);
    }

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
    (void)game;

    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);

    /* Grille */
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            char tile = game->map.grid[r][c];
            SDL_Rect dst = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            if (tile == TILE_WALL) {
                /* Mur bleu foncé avec bordure claire */
                SDL_SetRenderDrawColor(g_ren, 0, 0, 139, 255);
                SDL_RenderFillRect(g_ren, &dst);
                SDL_SetRenderDrawColor(g_ren, 33, 33, 255, 255);
                SDL_Rect border = { dst.x + 1, dst.y + 1,
                                    dst.w - 2, dst.h - 2 };
                SDL_RenderDrawRect(g_ren, &border);
            }
            else if (tile == TILE_PELLET) {
                /* Pastille ronde  */
                int cx = c * TILE_SIZE + TILE_SIZE / 2;
                int cy = r * TILE_SIZE + TILE_SIZE / 2;
                SDL_SetRenderDrawColor(g_ren, 255, 184, 151, 255); /* beige rosé */
                // SDL_Rect dot = { cx - 2, cy - 2, 4, 4 };
                // SDL_RenderFillRect(g_ren, &dot);
                 SDL_RenderDrawPoint(g_ren, cx,   cy);
                 SDL_RenderDrawPoint(g_ren, cx+1, cy);
                 SDL_RenderDrawPoint(g_ren, cx,   cy+1);
                 SDL_RenderDrawPoint(g_ren, cx+1, cy+1);
            }
            else if (tile == TILE_POWER_PELLET) {
                /* Super pastille — plus grande, clignotante */
                Uint32 t = SDL_GetTicks();
                if ((t / 300) % 2) {
                    int cx = c * TILE_SIZE + TILE_SIZE / 2;
                    int cy = r * TILE_SIZE + TILE_SIZE / 2;
                    SDL_SetRenderDrawColor(g_ren, 255, 184, 151, 255);
                    // SDL_Rect pp = { cx - 5, cy - 5, 10, 10 };
                    // SDL_RenderFillRect(g_ren, &pp);
                    int r2 = 4;
            for (int dy = -r2; dy <= r2; dy++)
                for (int dx = -r2; dx <= r2; dx++)
                    if (dx*dx + dy*dy <= r2*r2)
                        SDL_RenderDrawPoint(g_ren, cx + dx, cy + dy);
                    }
                }
            else if (tile == TILE_DOOR) {
                /* Porte de la cage — trait rose horizontal */
                SDL_SetRenderDrawColor(g_ren, 255, 182, 255, 255);
                SDL_Rect door = { dst.x, dst.y + TILE_SIZE/2 - 1,
                                  dst.w, 3 };
                SDL_RenderFillRect(g_ren, &door);
            }
        }
    }

    /* Pac-Man — cercle jaune avec bouche */
    Uint32 t = SDL_GetTicks();
    int mouth_open = (t / 150) % 2;

    int cx = game->player.entity.px + TILE_SIZE / 2;
    int cy = game->player.entity.py + TILE_SIZE / 2;
    int radius = TILE_SIZE / 2 - 1;

    SDL_SetRenderDrawColor(g_ren, 255, 255, 0, 255);

    /* Dessine un cercle rempli pixel par pixel */
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                /* Bouche ouverte : exclut un triangle vers la droite */
                if (mouth_open) {
                    int adx = dx, ady = dy;
                    /* adapte selon la direction */
                    Direction dir = game->player.entity.dir;
                    if (dir == DIR_LEFT)  adx = -dx;
                    if (dir == DIR_UP)  { adx = dy;  ady = -dx; }
                    if (dir == DIR_DOWN){ adx = -dy; ady = dx;  }
                    if (adx > 0 && abs(ady) < adx)
                        continue; /* zone bouche */
                }
                SDL_RenderDrawPoint(g_ren, cx + dx, cy + dy);
            }
        }
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