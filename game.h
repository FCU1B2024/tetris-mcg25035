#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <ncurses.h>
#include "shapes.h"

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

#define LEFT_KEY KEY_LEFT
#define RIGHT_KEY KEY_RIGHT
#define ROTATE_KEY 'z'
#define DOWN_KEY KEY_DOWN
#define FALL_KEY ' '

extern int key;
extern bool fallone;

#define LEFT_FUNC(key) ((key) == LEFT_KEY)
#define RIGHT_FUNC(key) ((key) == RIGHT_KEY)
#define ROTATE_FUNC(key) ((key) == ROTATE_KEY)
#define DOWN_FUNC(key) ((key) == DOWN_KEY)
#define FALL_FUNC(key) ((key) == FALL_KEY)

typedef struct {
    int x;
    int y;
    int score;
    int rotate;
    int fallTime;
    ShapeId queue[4];
    int leftKeyTime; // 左键按住时间
    int rightKeyTime; // 右键按住时间
    int damage;
    bool gameOver;
    bool win;
} State;

typedef struct {
    Color color;
    ShapeId shape;
    bool ghost;
    bool current;
} Block;

void setBlock(Block* block, Color color, ShapeId shape, bool current, bool ghost);
void resetBlock(Block* block);
void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state, int elapsedTime, bool* fullFall);
bool updateBlockPosition(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId);
int clearLine(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]);
bool isPositionAvailable(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int rotate, Shape shapeData, int size);
bool isBlockExist(Block block);
bool isFallingBlockExist(Block block);
void resetGhostBlock(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]);
void pushUpBlocks(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int height);
void sendCanvasToServer(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]);
void placeGhostBlockAtCoordinates(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int size, int rotate, Shape shapeData);

#endif
