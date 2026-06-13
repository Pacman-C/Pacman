#include "../include/game.h"
#include "../include/render.h"

#include <math.h>

void render_entities_init(void){}

static void render_pacman(const Game *game, Uint32 t)
{
    SDL_Renderer *ren = render_get_renderer();
    
    int cx     = (int)game->player.entity.px;
    int cy     = (int)game->player.entity.py;
    int radius = TILE_SIZE / 2 - 1;

    SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);

    if (game->state == STATE_PACMAN_DEAD) {
        Uint32 elapsed = SDL_GetTicks() - game->death_start;
        int angle = (elapsed * 90) / 800;  

        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    if (abs(dy) < dx * angle / 90)
                        continue;
                    SDL_RenderDrawPoint(ren, cx + dx, cy + dy);
                }
            }
        }
    } else {
        int mouth = (t / 150) % 2;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    if (mouth) {
                        int adx = dx, ady = dy;
                        Direction dir = game->player.entity.dir;
                        if (dir == DIR_LEFT)  adx = -dx;
                        if (dir == DIR_UP)  { adx = -dy; ady =  dx; }
                        if (dir == DIR_DOWN){ adx =  dy; ady = -dx; }
                        if (adx > 0 && abs(ady) < adx) continue;
                    }
                    SDL_RenderDrawPoint(ren, cx + dx, cy + dy);
                }
            }
        }
    }
}

static void render_ghost(const Game *game, int ghost_id, Uint32 t)
{
    SDL_Renderer *ren = render_get_renderer();
    SDL_Texture *sprite = render_get_sprite();
    
    const Ghost *g = &game->ghosts[ghost_id];

    SDL_Rect dst = {
        (int)g->entity.px,
        (int)g->entity.py,
        TILE_SIZE,
        TILE_SIZE
    };
    SDL_Rect src = {0, 0, TILE_SIZE, TILE_SIZE};

    if (g->mode == GHOST_SCATTER ||
        g->mode == GHOST_CHASE   ||
        g->mode == GHOST_LEAVING ||
        g->mode == GHOST_PEN)
    {
        src.x = g->id * TILE_SIZE;
        src.y = 0;
    }
    else if (g->mode == GHOST_FRIGHTENED)
    {
        Uint32 elapsed = t - game->frightened_start;
        Uint32 duration = FRIGHTENED_DURATION_LVL(game->level);

        Uint32 remaining = (elapsed < duration) ? (duration - elapsed) : 0;

        int blink = 0;

        if (remaining <= FRIGHTENED_FLASH)
        {
            blink = (t / 250) % 2;
        }

        src.y = TILE_SIZE;              
        src.x = blink ? TILE_SIZE : 0;  
    }
    else if (g->mode == GHOST_DEAD)
    {
        src.y = 2 * TILE_SIZE;
        if      (g->entity.dir == DIR_LEFT)  src.x = 0;
        else if (g->entity.dir == DIR_RIGHT) src.x = TILE_SIZE;
        else if (g->entity.dir == DIR_UP)    src.x = 2 * TILE_SIZE;
        else if (g->entity.dir == DIR_DOWN)  src.x = 3 * TILE_SIZE;
        else                                 src.x = 0;  
    }

    SDL_RenderCopy(ren, sprite, &src, &dst);
}

void render_entities_draw(const Game *game, Uint32 t)
{
    render_pacman(game, t);

    for (int i = 0; i < GHOST_COUNT; i++) {
        render_ghost(game, i, t);
    }
}