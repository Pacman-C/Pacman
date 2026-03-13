#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include "base.h"

void map_init(Map *map);
char get_tile(const Map *map, int x, int y);
void set_tile(Map *map, int x, int y, char tile);
bool is_walkable(const Map *map, int x, int y);

#endif /* MAP_H */


