#include "../include/game.h"
#include "../include/map.h"

#include <string.h>

#include "../include/base.h"
#include "../include/pacman.h"
#include "../include/ghosts.h"

void game_init(Game *game) {
    map_init(&game->map);
    game->player.score = 0;
    game->player.lives = 3;
    game->state = STATE_READY;
    game->level = 1;
    game->ghosts_eaten_combo = 0;
    game->frightened_start = 0;
    game->last_tick = SDL_GetTicks();
    game->high_score = 0;

    game->player.entity.x = 14; // Position de départ
    game->player.entity.y = 23;
    game->player.entity.px = 14 * TILE_SIZE + TILE_SIZE / 2;
    game->player.entity.py = 23 * TILE_SIZE + TILE_SIZE / 2;
    game->player.entity.dir = DIR_LEFT;
    game->player.entity.next_dir = DIR_LEFT;
    game->player.entity.speed = SPEED_PACMAN; // Vitesse de déplacement en pixels par seconde
    game->death_reset_done = 0;
    ghost_init(game->ghosts);

    game->scatter_chase_index = 0;
    game->scatter_chase_timer = SDL_GetTicks();
    
}

void game_update(Game *game, float delta) {

    if (game->state == STATE_PACMAN_DEAD && !game->death_reset_done)
    {
        game->player.lives--;
        if (game->player.lives <= 0)
        {
            game->state = STATE_GAMEOVER;
            return;
        }

        ghost_init(game->ghosts);

        game->player.entity.x = 14;
        game->player.entity.y = 23;
        game->player.entity.px = 14 * TILE_SIZE + TILE_SIZE / 2;
        game->player.entity.py = 23 * TILE_SIZE + TILE_SIZE / 2;
        game->death_reset_done = 1;
        game->state = STATE_PLAYING;
        game->player.entity.dir      = DIR_LEFT;
        game->player.entity.next_dir = DIR_NONE;
    }

    if (game->state != STATE_PLAYING) {
        return;
    }

    pacman_update(&game->player, &game->map, delta);
    ghost_update(game->ghosts, &game->map, &game->player, delta, game);

    if (game->map.pellet_count == 0)
    {
        game->level++;
        map_init(&game->map);
        game->player.entity.x = 14; // Position de départ
        game->player.entity.y = 23;
        game->player.entity.px = 14 * TILE_SIZE + TILE_SIZE / 2;
        game->player.entity.py = 23 * TILE_SIZE + TILE_SIZE / 2;
        ghost_init(game->ghosts);
    }

    if (game->player.is_powered)
    {
        for (int i = 0; i < GHOST_COUNT; i++)
        {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE)
            {
                g->mode_before_fright = g->mode;
                g->mode = GHOST_FRIGHTENED;
                g->entity.dir = opposite(g->entity.dir);
            }
        }
        game->frightened_start = SDL_GetTicks();
        game->player.is_powered = 0;
    }

    if (game->player.power_timer > 0)
    {
        Uint32 now = SDL_GetTicks();
        if (now - game->player.power_timer > FRIGHTENED_DURATION)
        {
            game->player.power_timer = 0;
            game->ghosts_eaten_combo = 0;
            for (int i = 0; i < GHOST_COUNT; i++)
            {
                Ghost *g = &game->ghosts[i];
                if (g->mode == GHOST_FRIGHTENED)
                {
                    g->mode = g->mode_before_fright;
                }
            }
        }
    }

    if (game->player.score >= EXTRA_LIFE_SCORE && game->player.lives == 3)
    {
        game->player.lives++;
    }
}

void handle_input(Game *game) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_UP]){
        pacman_set_dir(&game->player, DIR_UP);
        game->state = STATE_PLAYING;

    }
    if (keys[SDL_SCANCODE_DOWN]){
        pacman_set_dir(&game->player, DIR_DOWN);
        game->state = STATE_PLAYING;

    }
    if (keys[SDL_SCANCODE_LEFT]){
        pacman_set_dir(&game->player, DIR_LEFT);
        game->state = STATE_PLAYING;

    }
    if (keys[SDL_SCANCODE_RIGHT]){
        pacman_set_dir(&game->player, DIR_RIGHT);
        game->state = STATE_PLAYING;

    }

    if (game->state == STATE_READY) {
        game->state = STATE_PLAYING;
        game->death_reset_done = 0;
    }
}
        
