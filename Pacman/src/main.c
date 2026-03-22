#include "../include/base.h"
#include "../include/render.h"

int main(void)
{
    render_init();

    int running = 1;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE)
                running = 0;
        }
        render_frame(NULL); /* NULL ok tant que game n'est pas init */
    }

    render_quit();
    return 0;
}