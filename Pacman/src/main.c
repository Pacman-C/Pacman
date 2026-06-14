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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        return 1;
    }


    game_init(&game);
    render_init();
    audio_init();

    game.last_tick = SDL_GetTicks();

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

        if (game.state == STATE_GAMEOVER)
        {
            continue;
        }
    }

    audio_quit();
    render_quit();
    SDL_Quit();

    return 0;
}