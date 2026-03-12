#include "../include/render.h"

void render_frame(Game *game) { (void)game; }
void render_quit(void) {}

void render_init(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow(
        "Pac-Man",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        0
    );
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);

    SDL_Delay(2000);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    exit(0);  // ← termine proprement le process
}