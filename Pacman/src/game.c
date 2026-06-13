#include "../include/game.h"
#include "../include/map.h"
#include "../include/render.h"
#include "../include/audio.h"
#include <string.h>
#include "../include/base.h"
#include "../include/pacman.h"
#include "../include/ghosts.h"

void game_init(Game *game) {
    map_init(&game->map);
    game->player.score = 0;
    game->player.lives = 3;
    game->state = STATE_TITLE;      
    game->level = 1;
    game->ghosts_eaten_combo = 0;
    game->ghost_score_visible = 0;
    game->ghost_score_value = 0;
    game->frightened_start = 0;
    game->last_tick = SDL_GetTicks();
    game->high_score = 0;
    game->fruit_spawn_count = 0;
    game->fruit_active = 0;
    game->fruit_type = FRUIT_CHERRY;

    game->player.entity.x = 14;
    game->player.entity.y = 23;
    game->player.entity.px = 14 * TILE_SIZE + TILE_SIZE / 2;
    game->player.entity.py = 23 * TILE_SIZE + TILE_SIZE / 2;
    game->player.entity.dir = DIR_LEFT;
    game->player.entity.next_dir = DIR_NONE;
    game->player.entity.speed = SPEED_PACMAN;
    game->death_reset_done = 0;
    game->player.is_powered = 0;
    game->player.power_timer = 0;
    game->extra_life_earned = 0;
    game->death_start = 0;
    ghost_init(game->ghosts);

    game->scatter_chase_index = 0;
    game->scatter_chase_timer = SDL_GetTicks();

    game->level_transition_start = 0;
}

static FruitType fruit_for_level(int level)
{
    if (level == 1) return FRUIT_CHERRY;
    if (level == 2) return FRUIT_STRAWBERRY;
    if (level <= 4) return FRUIT_ORANGE;
    if (level <= 6) return FRUIT_APPLE;
    if (level <= 8) return FRUIT_MELON;
    if (level <= 10) return FRUIT_GALAXIAN;
    if (level <= 12) return FRUIT_BELL;
    return FRUIT_KEY;
}

static void game_try_spawn_fruit(Game *game)
{
    if (game->fruit_active)
        return;

    int eaten = game->map.total_pellets - game->map.pellet_count;

    if ((game->fruit_spawn_count == 0 && eaten >= FRUIT_SPAWN_1) ||
        (game->fruit_spawn_count == 1 && eaten >= FRUIT_SPAWN_2))
    {
        game->fruit_x      = 13;
        game->fruit_y      = 17;
        game->fruit_type   = fruit_for_level(game->level);
        game->fruit_active = 1;
        set_tile(&game->map, 13, 17, TILE_FRUIT);
        game->fruit_timer  = SDL_GetTicks();
        game->fruit_spawn_count++;
    }
}

static void game_update_fruit(Game *game)
{
    if (!game->fruit_active)
        return;

    if (SDL_GetTicks() - game->fruit_timer > FRUIT_DURATION)
    {
        char tile = get_tile(&game->map, game->fruit_x, game->fruit_y);
        if (tile == TILE_FRUIT)
            set_tile(&game->map, game->fruit_x, game->fruit_y, TILE_EMPTY);
        game->fruit_active = 0;
    }
}

static void game_update_frightened(Game *game)
{
    if (game->player.is_powered)
    {
        for (int i = 0; i < GHOST_COUNT; i++)
        {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_SCATTER || g->mode == GHOST_CHASE || g->mode == GHOST_PEN)
            {
                g->mode_before_fright = g->mode;
                g->mode = GHOST_FRIGHTENED;
                g->entity.dir = opposite(g->entity.dir);
            }
        }
        game->frightened_start = SDL_GetTicks();
        game->player.is_powered = 0;
        audio_play(SOUND_POWER_UP);     
    }

    if (game->player.power_timer > 0)
    {
        Uint32 now = SDL_GetTicks();
        if (now - game->player.power_timer > FRIGHTENED_DURATION_LVL(game->level))
        {
            game->player.power_timer = 0;
            game->ghosts_eaten_combo = 0;
            for (int i = 0; i < GHOST_COUNT; i++)
            {
                Ghost *g = &game->ghosts[i];
                if (g->mode == GHOST_FRIGHTENED)
                    g->mode = g->mode_before_fright;
            }
        }
    }
}

static void game_reset_player(Game *game)
{
    game->player.entity.x        = 14;
    game->player.entity.y        = 23;
    game->player.entity.px       = 14 * TILE_SIZE + TILE_SIZE / 2;
    game->player.entity.py       = 23 * TILE_SIZE + TILE_SIZE / 2;
    game->player.entity.dir      = DIR_LEFT;
    game->player.entity.next_dir = DIR_NONE;
    game->player.is_powered = 0;
    game->player.power_timer = 0;
}

void game_update(Game *game, float delta)
{
    if (game->ghost_score_visible)
    {
        if (SDL_GetTicks() - game->ghost_score_timer > 1000)
        {
            game->ghost_score_visible = 0;
        }
    }
    
    if (game->state == STATE_PACMAN_DEAD && !game->death_reset_done)
    {
        game->death_reset_done = 1;

        audio_play(SOUND_PACMAN_DEATH);
        game->player.lives--;

        if (game->player.lives <= 0)
        {
            game->state = STATE_GAMEOVER;
            audio_play(SOUND_GAME_OVER);
        }
        else
        {
            ghost_init(game->ghosts);
            game->fruit_active = 0;
            game_reset_player(game);
            game->state = STATE_PLAYING;
            game->death_reset_done = 0;
        }
    }

    if (game->state == STATE_LEVEL_TRANSITION)
    {
        Uint32 elapsed = SDL_GetTicks() - game->level_transition_start;

        if (elapsed > 4000)
        {
            game_start_next_level(game);
        }

        return;
    }
    if (game->state != STATE_PLAYING) {
        return;
    }

    pacman_update(&game->player, &game->map, delta, game);
    ghost_update(game->ghosts, &game->map, &game->player, delta, game);

    if (game->map.pellet_count == 0)
    {
        game->state = STATE_LEVEL_TRANSITION;
        game->level_transition_start = SDL_GetTicks();
        audio_play(SOUND_LEVEL_COMPLETE);
        return;
    }

    game_update_frightened(game);

    if (game->player.score >= EXTRA_LIFE_SCORE && !game->extra_life_earned)
    {
        game->player.lives++;
        game->extra_life_earned = 1;
    }

    game_try_spawn_fruit(game);
    game_update_fruit(game);

}


void game_pacman_ate_pellet(Game *game)
{
    (void)game;
}

void game_pacman_ate_ghost(Game *game)
{
    (void)game;
}


void game_start_next_level(Game *game)
{
    game->level++;

    map_init(&game->map);

    game_reset_player(game);

    ghost_init(game->ghosts);

    game->fruit_spawn_count = 0;
    game->fruit_active = 0;
    game->extra_life_earned = 0;
    game->ghosts_eaten_combo = 0;

    game->state = STATE_PLAYING;
    game->level_transition_start = 0;

    audio_play(SOUND_INTERMISSION);
}

void handle_input(Game *game)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_UP]) {
        pacman_set_dir(&game->player, DIR_UP);
        if (game->state == STATE_TITLE)
            game->state = STATE_PLAYING;
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        pacman_set_dir(&game->player, DIR_DOWN);
        if (game->state == STATE_TITLE)
            game->state = STATE_PLAYING;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
        pacman_set_dir(&game->player, DIR_LEFT);
        if (game->state == STATE_TITLE)
            game->state = STATE_PLAYING;
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        pacman_set_dir(&game->player, DIR_RIGHT);
        if (game->state == STATE_TITLE)
            game->state = STATE_PLAYING;
    }

    if (keys[SDL_SCANCODE_SPACE]) {
        if (game->state == STATE_TITLE) {
            game->state = STATE_PLAYING;
            game->death_reset_done = 0;
        }
        else if (game->state == STATE_GAMEOVER) {
            game_init(game);
        }
    }
}