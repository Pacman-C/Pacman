#include "../include/game.h"
#include "../include/render.h"

void render_map_draw(const Game *game, Uint32 t, int flash)
{
    SDL_Renderer *ren = render_get_renderer();
    SDL_Texture *sprite = render_get_sprite();

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            char tile = game->map.grid[r][c];
            SDL_Rect dst = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            if (tile == TILE_WALL) {
                if (flash) {
                    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                } else {
                    SDL_SetRenderDrawColor(ren, 0, 0, 139, 255);
                }
                SDL_RenderFillRect(ren, &dst);
            }
            else if (tile == TILE_PELLET) {
                int cx = c * TILE_SIZE + TILE_SIZE / 2;
                int cy = r * TILE_SIZE + TILE_SIZE / 2;
                SDL_SetRenderDrawColor(ren, 255, 184, 151, 255);
                SDL_RenderDrawPoint(ren, cx, cy);
            }
            else if (tile == TILE_POWER_PELLET) {
                if ((t / 300) % 2) {
                    int cx = c * TILE_SIZE + TILE_SIZE / 2;
                    int cy = r * TILE_SIZE + TILE_SIZE / 2;
                    SDL_SetRenderDrawColor(ren, 255, 184, 151, 255);
                    SDL_Rect p = { cx - 2, cy - 2, 4, 4 };
                    SDL_RenderFillRect(ren, &p);
                }
            }
        }
    }

    /* Rendu des fruits */
    if (game->fruit_active)
    {
        SDL_Rect dst = {game->fruit_x * TILE_SIZE, game->fruit_y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

        SDL_Rect src;
        src.y = 0;  /* les fruits sont sur la ligne 0, à droite des fantômes */
        src.w = TILE_SIZE;
        src.h = TILE_SIZE;

        switch (game->fruit_type)
        {
            case FRUIT_CHERRY:     src.x = 4 * TILE_SIZE; break;  /* x=64 */
            case FRUIT_STRAWBERRY: src.x = 5 * TILE_SIZE; break;  /* x=80 */
            case FRUIT_ORANGE:     src.x = 6 * TILE_SIZE; break;  /* x=96 */
            case FRUIT_APPLE:      src.x = 7 * TILE_SIZE; break;  /* x=112 */
            case FRUIT_MELON:      src.x = 8 * TILE_SIZE; break;  /* x=128 */
            case FRUIT_GALAXIAN:   src.x = 9 * TILE_SIZE; break;  /* x=144 */
            case FRUIT_BELL:       src.x = 10 * TILE_SIZE; break; /* x=160 */
            case FRUIT_KEY:        src.x = 11 * TILE_SIZE; break; /* x=176 */
        }

        SDL_RenderCopy(ren, sprite, &src, &dst);
    }
}