#include "../include/base.h"
#include "../include/render.h"
#include "../include/game.h"
#include <SDL2/SDL.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Game game;
    SDL_Event e;
    int running = 1;

    srand(time(NULL));

    /* SDL INIT */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL initialisé OK\n");

    game_init(&game);
    render_init();
    audio_init();

    game.last_tick = SDL_GetTicks();

    /* TEST AUDIO */
    audio_play(SOUND_CHOMP);

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        running = 0;
                        break;

                    case SDLK_d:
                        render_toggle_debug();
                        break;

                    case SDLK_SPACE:
                        audio_play(SOUND_CHOMP);
                        break;

                    /* IMPORTANT: restart game */
                    case SDLK_r:
                        game_init(&game);
                        printf("Game restarted\n");
                        break;

                    default:
                        break;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        float delta = (now - game.last_tick) / 1000.0f;
        game.last_tick = now;

        handle_input(&game);
        game_update(&game, delta);
        render_frame(&game);

        /* safety : si GAME OVER, tu peux choisir de freeze ou restart */
        if (game.state == STATE_GAMEOVER)
        {
            // option 1 : freeze
            continue;

            // option 2 : auto restart (décommenter si tu veux)
            // game_init(&game);
        }
    }

    audio_quit();
    render_quit();
    SDL_Quit();

    return 0;
}