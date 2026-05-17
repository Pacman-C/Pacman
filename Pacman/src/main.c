#include "../include/base.h"
#include "../include/render.h"
#include "../include/game.h"
#include <SDL2/SDL.h>

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game     game;
    SDL_Event e;
    int      running = 1;

    game_init(&game);
    render_init();
    game.last_tick = SDL_GetTicks();

    while (running && game.state != STATE_GAMEOVER)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                    case SDLK_d:
                        render_toggle_debug();   /* ← touche D */
                        break;
                    default:
                        break;
                }
            }
        }

        Uint32 now   = SDL_GetTicks();
        float  delta = (now - game.last_tick) / 1000.0f;
        game.last_tick = now;

        handle_input(&game);
        game_update(&game, delta);

        render_frame(&game);
    }

    render_quit();
    return 0;
}