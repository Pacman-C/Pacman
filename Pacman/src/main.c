#include "game.h"

int main() {
    Game game;
    game_init(&game);
    render_init();

    while (game.state != STATE_GAMEOVER) {
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