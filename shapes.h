#ifndef SHAPES_H
#define SHAPES_H

typedef enum {
    RED = 41,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
    BLACK = 0,
} Color;

typedef enum {
    EMPTY = -1,
    I,
    J,
    L,
    O,
    S,
    T,
    Z
} ShapeId;

typedef struct {
    ShapeId shape;
    Color color;
    int size;
    char rotates[4][4][4];
} Shape;

extern Shape shapes[7];

#endif
