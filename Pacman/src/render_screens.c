#include "../include/render_screens.h"
#include "../include/render.h"
#include "../include/base.h"
#include "../include/font.h"
#include "../include/game.h"

#define FONT_W 4

static int text_width(const char *s)
{
    int len = 0;
    while (s[len]) len++;
    return len * FONT_W;
}

static int center_x(const char *s)
{
    return (WINDOW_W - text_width(s)) / 2;
}

static void draw_char_screen(int x, int y, char c, SDL_Color col)
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

static void draw_string_screen(int x, int y, const char *s, SDL_Color col)
{
    for (int i = 0; s[i]; i++)
        draw_char_screen(x + i * FONT_W, y, s[i], col);
}

void render_screen_title(void)
{
    SDL_Renderer *ren = render_get_renderer();

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color cyan   = {0, 222, 222, 255};

    draw_string_screen(center_x("PAC-MAN"), 60,
                       "PAC-MAN", yellow);

    draw_string_screen(center_x("PRESS SPACE TO START"),
                       WINDOW_H / 2,
                       "PRESS SPACE TO START",
                       white);

    draw_string_screen(10,
                       WINDOW_H - 30,
                       "NAMCO 1980",
                       cyan);

    SDL_RenderPresent(ren);
}

void render_screen_game_over(int final_score)
{
    SDL_Renderer *ren = render_get_renderer();

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    SDL_Color red    = {255, 0, 0, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color white  = {255, 255, 255, 255};

    draw_string_screen(center_x("GAME OVER"),
                       WINDOW_H / 2 - 50,
                       "GAME OVER",
                       red);

    draw_string_screen(center_x("SCORE"),
                       WINDOW_H / 2 + 10,
                       "SCORE",
                       yellow);

    char score_buf[32];
    SDL_snprintf(score_buf, sizeof(score_buf), "%d", final_score);

    draw_string_screen(center_x(score_buf),
                       WINDOW_H / 2 + 25,
                       score_buf,
                       yellow);

    draw_string_screen(center_x("PRESS SPACE TO RETRY"),
                       WINDOW_H / 2 + 60,
                       "PRESS SPACE TO RETRY",
                       white);

    SDL_RenderPresent(ren);
}

void render_screen_victory(int level)
{
    SDL_Renderer *ren = render_get_renderer();

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    SDL_Color cyan   = {0, 222, 222, 255};
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    draw_string_screen(center_x("LEVEL CLEAR"),
                       WINDOW_H / 2 - 50,
                       "LEVEL CLEAR",
                       cyan);

    draw_string_screen(center_x("NEXT LEVEL"),
                       WINDOW_H / 2 + 10,
                       "NEXT LEVEL",
                       white);

    char level_buf[32];
    SDL_snprintf(level_buf, sizeof(level_buf),
                 "LEVEL %d", level + 1);

    draw_string_screen(center_x(level_buf),
                       WINDOW_H / 2 + 30,
                       level_buf,
                       yellow);

    draw_string_screen(center_x("PRESS SPACE TO CONTINUE"),
                       WINDOW_H / 2 + 70,
                       "PRESS SPACE TO CONTINUE",
                       white);

    SDL_RenderPresent(ren);
}

void render_screen_level_transition(const Game *game)
{
    SDL_Renderer *ren = render_get_renderer();

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    Uint32 t = SDL_GetTicks();
    int flash = (t / 200) % 2;

    render_map_draw(game, t, flash);

    SDL_Color cyan  = {0, 222, 222, 255};
    SDL_Color white = {255, 255, 255, 255};

    draw_string_screen(center_x("GET READY"),
                       WINDOW_H / 2 - 60,
                       "GET READY",
                       cyan);

    char level_buf[32];
    SDL_snprintf(level_buf,
                 sizeof(level_buf),
                 "LEVEL %d",
                 game->level);

    draw_string_screen(center_x(level_buf),
                       WINDOW_H / 2 + 40,
                       level_buf,
                       white);

    SDL_RenderPresent(ren);
}