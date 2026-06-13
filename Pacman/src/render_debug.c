#include "../include/render.h"
#include "../include/game.h"
#include "../include/font.h"

static int g_debug_enabled = 0;


static const SDL_Color DEBUG_COLORS[GHOST_COUNT] = {
    {222,   0,   0, 180},   /* Red */
    {255, 184, 222, 180},   /* Pink */
    {  0, 222, 222, 180},   /* Cyan */
    {222, 133,   0, 180},   /* Orange */
};

static const char *MODE_NAMES[] = {
    "SCATTER", "CHASE", "FRIGHTENED", "DEAD", "PEN", "LEAVING"
};


void render_debug_init(void)
{
    g_debug_enabled = 0;
}

void render_debug_set_enabled(int enabled)
{
    g_debug_enabled = enabled;
}


static void draw_char(int x, int y, char c, SDL_Color col)
{
    SDL_Renderer *ren = render_get_renderer();
    int idx = font_idx(c);
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int row = 0; row < 5; row++) {
        Uint8 bits = FONT[idx][row];
        for (int col2 = 0; col2 < 3; col2++) {
            if (bits & (1 << (2 - col2)))
                SDL_RenderDrawPoint(ren, x + col2, y + row);
        }
    }
}

static void draw_string(int x, int y, const char *s, SDL_Color col)
{
    for (int i = 0; s[i]; i++)
        draw_char(x + i * 4, y, s[i], col);
}

static void draw_target_tile(int tx, int ty, SDL_Color col)
{
    SDL_Renderer *ren = render_get_renderer();
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    SDL_Rect r = { tx * TILE_SIZE + 2, ty * TILE_SIZE + 2,
                   TILE_SIZE - 4,      TILE_SIZE - 4 };
    SDL_RenderFillRect(ren, &r);
  
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, 255);
    SDL_RenderDrawRect(ren, &r);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
}

void render_debug_draw_targets(const Game *game)
{
    if (!g_debug_enabled) return;

    for (int i = 0; i < GHOST_COUNT; i++) {
        const Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_SCATTER ||
            g->mode == GHOST_CHASE   ||
            g->mode == GHOST_DEAD)
        {
            draw_target_tile(g->target_x, g->target_y, DEBUG_COLORS[i]);
        }
    }
}

void render_debug_draw_ghost_modes(const Game *game)
{
    if (!g_debug_enabled) return;

    for (int i = 0; i < GHOST_COUNT; i++) {
        const Ghost *g = &game->ghosts[i];
        const char *mname = MODE_NAMES[g->mode];
        SDL_Color col = DEBUG_COLORS[i];
        col.a = 255;
        draw_string((int)g->entity.px,
                    (int)g->entity.py + TILE_SIZE + 1,
                    mname, col);
    }
}

void render_debug_draw_phase(const Game *game, Uint32 t)
{
    if (!g_debug_enabled) return;

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

void render_debug_draw_lives(const Game *game)
{
    SDL_Renderer *ren = render_get_renderer();
    int r2 = 5;
    for (int i = 0; i < game->player.lives; i++) {
        int lx = 12 + i * 14;
        int ly = WINDOW_H - 8;
        SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
        for (int dy = -r2; dy <= r2; dy++)
            for (int dx = -r2; dx <= r2; dx++)
                if (dx*dx + dy*dy <= r2*r2)
                    SDL_RenderDrawPoint(ren, lx + dx, ly + dy);
    }
}

void render_debug_draw_score(const Game *game)
{
    char score_buf[32];
    SDL_snprintf(score_buf, sizeof(score_buf), "%d", game->player.score);
    SDL_Color yellow = {255, 255, 0, 255};
    draw_string(WINDOW_W / 2 - 20, WINDOW_H - 12, score_buf, yellow);
}

void render_debug_draw_ghost_score(const Game *game)
{
    if (!game->ghost_score_visible) return;

    char buf[8];
    SDL_snprintf(
        buf,
        sizeof(buf),
        "%d",
        game->ghost_score_value
    );

    SDL_Color white = {255, 255, 255, 255};
    draw_string(
        game->ghost_score_x * TILE_SIZE,
        game->ghost_score_y * TILE_SIZE,
        buf,
        white
    );
}