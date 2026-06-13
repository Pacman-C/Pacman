#ifndef ENTITIES_H
#define ENTITIES_H

#include "types.h"
#include "constants.h"

typedef struct {
    int       x, y;
    float     px, py;
    Direction dir;
    Direction next_dir;
    float     speed;
} Entity;

#endif /* ENTITIES_H */