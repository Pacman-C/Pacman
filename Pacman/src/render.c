#include "../include/render.h"
#include <stdio.h>
#include <stdlib.h>

static SDL_Window   *g_win    = NULL;
static SDL_Renderer *g_ren    = NULL;
static SDL_Texture  *g_sprite = NULL;

static int g_debug = 0;

static const SDL_Color DEBUG_COLORS[GHOST_COUNT] = {
    {222,   0,   0, 180},   
    {255, 184, 222, 180},   
    {  0, 222, 222, 180},   
    {222, 133,   0, 180},  
};

static const char *MODE_NAMES[] = {
    "SCATTER", "CHASE", "FRIGHTENED", "DEAD", "PEN", "LEAVING"
};


static void draw_target_tile(int tx, int ty, SDL_Color col)
{
    SDL_SetRenderDrawBlendMode(g_ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g_ren, col.r, col.g, col.b, col.a);
    SDL_Rect r = { tx * TILE_SIZE + 2, ty * TILE_SIZE + 2,
                   TILE_SIZE - 4,      TILE_SIZE - 4 };
    SDL_RenderFillRect(g_ren, &r);
  
    SDL_SetRenderDrawColor(g_ren, col.r, col.g, col.b, 255);
    SDL_RenderDrawRect(g_ren, &r);
    SDL_SetRenderDrawBlendMode(g_ren, SDL_BLENDMODE_NONE);
}

static const Uint8 FONT[][5] = {
/*  0 */ {0x6,0x5,0x5,0x5,0x6},
/*  1 */ {0x2,0x6,0x2,0x2,0x7},
/*  2 */ {0x6,0x1,0x6,0x4,0x7},
/*  3 */ {0x7,0x1,0x3,0x1,0x6},
/*  4 */ {0x5,0x5,0x7,0x1,0x1},
/*  5 */ {0x7,0x4,0x6,0x1,0x6},
/*  6 */ {0x3,0x4,0x6,0x5,0x6},
/*  7 */ {0x7,0x1,0x2,0x2,0x2},
/*  8 */ {0x6,0x5,0x6,0x5,0x6},
/*  9 */ {0x6,0x5,0x7,0x1,0x6},
/* A */ {0x2,0x5,0x7,0x5,0x5},
/* B */ {0x6,0x5,0x6,0x5,0x6},
/* C */ {0x3,0x4,0x4,0x4,0x3},
/* D */ {0x6,0x5,0x5,0x5,0x6},
/* E */ {0x7,0x4,0x6,0x4,0x7},
/* F */ {0x7,0x4,0x6,0x4,0x4},
/* G */ {0x3,0x4,0x5,0x5,0x3},
/* H */ {0x5,0x5,0x7,0x5,0x5},
/* I */ {0x7,0x2,0x2,0x2,0x7},
/* K */ {0x5,0x6,0x4,0x6,0x5},
/* L */ {0x4,0x4,0x4,0x4,0x7},
/* N */ {0x5,0x7,0x7,0x5,0x5},
/* R */ {0x6,0x5,0x6,0x6,0x5},
/* S */ {0x3,0x4,0x2,0x1,0x6},
/* T */ {0x7,0x2,0x2,0x2,0x2},
/* W */ {0x5,0x5,0x7,0x7,0x5},
/* Y */ {0x5,0x5,0x2,0x2,0x2},
/* ' '*/ {0x0,0x0,0x0,0x0,0x0},
};

static int font_idx(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    switch(c) {
        case 'A': return 10; case 'C': return 12; case 'D': return 13;
        case 'E': return 14; case 'F': return 15; case 'G': return 16;
        case 'H': return 17; case 'I': return 18; case 'K': return 19;
        case 'L': return 20; case 'N': return 21; case 'R': return 22;
        case 'S': return 23; case 'T': return 24; case 'W': return 25;
        case 'Y': return 26; case ' ': return 27;
        default:  return 27;
    }
}

static void draw_char(int x, int y, char c, SDL_Color col)
{
    int idx = font_idx(c);
    SDL_SetRenderDrawColor(g_ren, col.r, col.g, col.b, col.a);
    for (int row = 0; row < 5; row++) {
        Uint8 bits = FONT[idx][row];
        for (int col2 = 0; col2 < 3; col2++) {
            if (bits & (1 << (2 - col2)))
                SDL_RenderDrawPoint(g_ren, x + col2, y + row);
        }
    }
}

static void draw_string(int x, int y, const char *s, SDL_Color col)
{
    for (int i = 0; s[i]; i++)
        draw_char(x + i * 4, y, s[i], col);
}


void render_toggle_debug(void)
{
    g_debug = !g_debug;
}


void render_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
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
}

void render_frame(const Game *game)
{
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);

    Uint32 t     = SDL_GetTicks();
    int    flash = (game->state == STATE_WIN) ? (t / 200) % 2 : 0;

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            char tile = game->map.grid[r][c];
            SDL_Rect dst = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            if (tile == TILE_WALL) {
                if (flash) {
                    SDL_SetRenderDrawColor(g_ren, 255, 255, 255, 255);
                } else {
                    SDL_SetRenderDrawColor(g_ren, 0, 0, 139, 255);
                }
                SDL_RenderFillRect(g_ren, &dst);
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
            if (game->fruit_active &&
    r == game->fruit_y &&
    c == game->fruit_x)
    {
        SDL_Rect dst = {c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE};

        SDL_Rect src;
        src.y = 0;  // les fruits sont sur la ligne 0, à droite des fantômes
        src.w = TILE_SIZE;
        src.h = TILE_SIZE;

        switch (game->fruit_type)
        {
            case FRUIT_CHERRY:     src.x = 4 * TILE_SIZE; break; // x=64
            case FRUIT_STRAWBERRY: src.x = 5 * TILE_SIZE; break; // x=80
            case FRUIT_ORANGE:     src.x = 6 * TILE_SIZE; break; // x=96
            case FRUIT_APPLE:      src.x = 7 * TILE_SIZE; break; // x=112
            case FRUIT_MELON:      src.x = 8 * TILE_SIZE; break; // x=128
            case FRUIT_GALAXIAN:   src.x = 9 * TILE_SIZE; break; // x=144
            case FRUIT_BELL:       src.x = 10 * TILE_SIZE; break;// x=160
            case FRUIT_KEY:        src.x = 11 * TILE_SIZE; break;// x=176
        }

        SDL_RenderCopy(g_ren, g_sprite, &src, &dst);
    }
                 
        }
    }

    if (g_debug) {
        for (int i = 0; i < GHOST_COUNT; i++) {
            const Ghost *g = &game->ghosts[i];
            /* Afficher la cible seulement en mode actif */
            if (g->mode == GHOST_SCATTER ||
                g->mode == GHOST_CHASE   ||
                g->mode == GHOST_DEAD)
            {
                draw_target_tile(g->target_x, g->target_y, DEBUG_COLORS[i]);
            }
        }
    }

    {
        int cx     = (int)game->player.entity.px;
int cy     = (int)game->player.entity.py;
int radius = TILE_SIZE / 2 - 1;

SDL_SetRenderDrawColor(g_ren, 255, 255, 0, 255);

if (game->state == STATE_PACMAN_DEAD) {
    /* Bouche qui s'ouvre progressivement jusqu'à disparaître */
    Uint32 elapsed = SDL_GetTicks() - game->frightened_start;
    int angle = (elapsed * 90) / 800;  /* 0 à 90 degrés en 800ms */
    if (angle > 90) angle = 90;

    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                /* Exclure un triangle qui grandit */
                if (abs(dy) < dx * angle / 90)
                    continue;
                SDL_RenderDrawPoint(g_ren, cx + dx, cy + dy);
            }
        }
    }
} else {
    /* Animation normale */
    int mouth = (t / 150) % 2;
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                if (mouth) {
                    int adx = dx, ady = dy;
                    Direction dir = game->player.entity.dir;
                    if (dir == DIR_LEFT)  adx = -dx;
                    if (dir == DIR_UP)  { adx = -dy; ady =  dx; }
                    if (dir == DIR_DOWN){ adx =  dy; ady = -dx; }
                    if (adx > 0 && abs(ady) < adx) continue;
                }
                SDL_RenderDrawPoint(g_ren, cx + dx, cy + dy);
            }
        }
    }
}
    }

    for (int i = 0; i < GHOST_COUNT; i++) {
        const Ghost *g = &game->ghosts[i];

        SDL_Rect dst = {
            (int)g->entity.px,
            (int)g->entity.py,
            TILE_SIZE,
            TILE_SIZE
        };
        SDL_Rect src = {0, 0, TILE_SIZE, TILE_SIZE};

        if (g->mode == GHOST_SCATTER ||
            g->mode == GHOST_CHASE   ||
            g->mode == GHOST_LEAVING ||
            g->mode == GHOST_PEN)
        {
            src.x = g->id * TILE_SIZE;
            src.y = 0;
        }
        else if (g->mode == GHOST_FRIGHTENED)
        {
            Uint32 elapsed = t - game->frightened_start;
            Uint32 duration = FRIGHTENED_DURATION_LVL(game->level);

            Uint32 remaining = (elapsed < duration) ? (duration - elapsed) : 0;

            int blink = 0;

            if (remaining <= FRIGHTENED_FLASH)
            {
                blink = (t / 250) % 2;
            }

            src.y = TILE_SIZE;              
            src.x = blink ? TILE_SIZE : 0;  
        }
        else if (g->mode == GHOST_DEAD)
        {
            src.y = 2 * TILE_SIZE;
            if      (g->entity.dir == DIR_LEFT)  src.x = 0;
            else if (g->entity.dir == DIR_RIGHT) src.x = TILE_SIZE;
            else if (g->entity.dir == DIR_UP)    src.x = 2 * TILE_SIZE;
            else                                 src.x = 3 * TILE_SIZE;
        }

        SDL_RenderCopy(g_ren, g_sprite, &src, &dst);

        if (g_debug) {
            const char *mname = MODE_NAMES[g->mode];
            SDL_Color   col   = DEBUG_COLORS[i];
            col.a = 255;
            draw_string((int)g->entity.px,
                        (int)g->entity.py + TILE_SIZE + 1,
                        mname, col);
        }
    }

  
    if (g_debug) {
        Uint32 phase_elapsed = (t - game->scatter_chase_timer) / 1000;

       
        static const Uint32 PHASE_DURATIONS[] = {7, 20, 7, 20, 5, 20, 5, 0};
        int   idx      = game->scatter_chase_index;
        int   is_chase = (idx % 2 == 1);
        Uint32 dur     = PHASE_DURATIONS[idx < 8 ? idx : 7];

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color cyan  = {0, 222, 222, 255};
        SDL_Color red   = {222, 0, 0, 255};

        draw_string(2, WINDOW_H - 20,
                    is_chase ? "CHASE" : "SCATTER",
                    is_chase ? red : cyan);

        char buf[16];
        if (dur > 0)
            SDL_snprintf(buf, sizeof(buf), "%02d/%02ds", phase_elapsed, dur);
        else
            SDL_snprintf(buf, sizeof(buf), "%02ds INF", phase_elapsed);

        draw_string(2, WINDOW_H - 12, buf, white);
    }

    {
        int r2 = 5;
        for (int i = 0; i < game->player.lives; i++) {
            int lx = 12 + i * 14;
            int ly = WINDOW_H - 8;
            SDL_SetRenderDrawColor(g_ren, 255, 255, 0, 255);
            for (int dy = -r2; dy <= r2; dy++)
                for (int dx = -r2; dx <= r2; dx++)
                    if (dx*dx + dy*dy <= r2*r2)
                        SDL_RenderDrawPoint(g_ren, lx + dx, ly + dy);
        }
    }
    char score_buf[32];
    SDL_snprintf(score_buf, sizeof(score_buf), "%d", game->player.score);
    SDL_Color yellow = {255, 255, 0, 255};
    draw_string(WINDOW_W / 2 - 20, WINDOW_H - 12, score_buf, yellow);

    if (game->ghost_score_visible)
    {
        char buf[8];

        SDL_snprintf(
            buf,
            sizeof(buf),
            "%d",
            game->ghost_score_value
        );

        SDL_Color white = {255,255,255,255};

        draw_string(
            game->ghost_score_x * TILE_SIZE,
            game->ghost_score_y * TILE_SIZE,
            buf,
            white
        );
    }
    SDL_RenderPresent(g_ren);
}


void render_quit(void)
{
    if (g_sprite) SDL_DestroyTexture(g_sprite);
    if (g_ren)    SDL_DestroyRenderer(g_ren);
    if (g_win)    SDL_DestroyWindow(g_win);
    SDL_Quit();
}