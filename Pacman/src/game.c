#include "../include/game.h"
#include "../include/map.h"
#include <string.h>
#include "../include/base.h"

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
    game->player.entity.px = 14 * TILE_SIZE; // Position pixel pour le rendu
    game->player.entity.py = 23 * TILE_SIZE;
    game->player.entity.dir = DIR_LEFT;
    game->player.entity.next_dir = DIR_LEFT;
    game->player.entity.speed = SPEED_PACMAN; // Vitesse de déplacement en pixels par seconde

    //@TODO: Initialisation des fantômes (position, mode, etc.)
}

void game_update(Game *game, float delta) { (void)game; (void)delta; }
void handle_input(Game *game) { (void)game; }