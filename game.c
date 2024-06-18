#include "game.h"
#include "shapes.h"
#include <inttypes.h>
#include <time.h>

int64_t lastFallTime = 0;
bool gameOver = false;

int64_t millis(){
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return ((int64_t) now.tv_sec) * 1000 + ((int64_t) now.tv_nsec) / 1000000;
}

void setBlock(Block* block, Color color, ShapeId shape, bool current, bool ghost){
    block->color = color;
    block->shape = shape;
    block->current = current;
    block->ghost = ghost;
}

void resetBlock(Block* block){
    block->color = BLACK;
    block->shape = EMPTY;
    block->current = false;
    block->ghost = false;
}

void falledEvent(State* state, Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]){
    int clearedLine = clearLine(canvas);
    if (clearedLine == 0) state->score = 0;
    state->score += clearedLine;
    if (state -> score > 12) state -> score = 12;
    
    if (state->damage > state->score) {
        state->score = 0;
        state->damage = state->damage - state->score;
    }
    else if (state->damage < state->score) {
        state->score = state->score - state->damage;
        state->damage = 0;
    }
    else if (state->damage == state->score) {
        state->score = 0;
        state->damage = 0;
    }

    if (state->damage > 0) {
        pushUpBlocks(canvas, state->damage);
        state->damage = 0;
    }

    sendAttack(state->score);
    state->x = CANVAS_WIDTH / 2;
    state->y = 0;
    state->rotate = 0;
    state->queue[0] = state->queue[1];
    state->queue[1] = state->queue[2];
    state->queue[2] = state->queue[3];
    state->queue[3] = rand() % 7;
    if (!updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x, state->y, state->rotate, state->queue[0])) {
        gameOver = true;
    }
}

bool isKeyMovingPreventFall(){
    if (lastFallTime == 0) return false;
    int64_t currentTime = millis();
    return currentTime - lastFallTime < 300;
}

void updateLastKeyTime(){
    lastFallTime = millis();
}

void sendCanvasToServer(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]){
    char message[1024] = "";
    int index = 0;
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            Color color = canvas[i][j].color;
            bool isGhost = canvas[i][j].ghost;
            if (color == BLACK || isGhost) {
                message[index] = '0';
            }
            else if (color == RED) {
                message[index] = 'R';
            }
            else if (color == GREEN) {
                message[index] = 'G';
            }
            else if (color == YELLOW) {
                message[index] = 'Y';
            }
            else if (color == BLUE) {
                message[index] = 'B';
            }
            else if (color == CYAN) {
                message[index] = 'C';
            }
            else if (color == WHITE) {
                message[index] = 'W';
            }
            else if (color == PURPLE) {
                message[index] = 'P';
            }
            else if (color == WHITE) {
                message[index] = 'W';
            }
            index++;
        }
        message[index] = '*';
        index++;
    }
    message[index] = '\0';
    if (gameOver) {
        send_to_server("GAMEOVER");
        return;
    }
    send_to_server(message);
}

void sendAttack(int attack){
    char message[1024] = "";
    sprintf(message, "attackDataJson{\"attack\":%d}attackDataJson\n", attack);
    send_to_server(message);
}

void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state, int elapsedTime, bool* fullFall) {
    key = getch();
    bool moved = false;
    printw("%c", key);
    if (gameOver) {
        state->gameOver = true;
        return;
    }
    if (ROTATE_FUNC(key)) {
        updateLastKeyTime();
        int newRotate = (state->rotate + 1) % 4;
        if ((updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x, state->y, newRotate, state->queue[0]))) {
            state->rotate = newRotate;
        }
    } else if (LEFT_FUNC(key)) {
        updateLastKeyTime();
        if ((updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x-1, state->y, state->rotate, state->queue[0]))) {
            state->x -= 1;
            moved = true;
            state->leftKeyTime = 0; 
        }
    } else if (RIGHT_FUNC(key)) { 
        updateLastKeyTime();
        if (updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x+1, state->y, state->rotate, state->queue[0])) {
            state->x += 1;
            moved = true;
            state->rightKeyTime = 0;
        }
    } else if (DOWN_FUNC(key)) {
        updateLastKeyTime();
        if (updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x, state->y+1, state->rotate, state->queue[0])) {
            state->y += 1;
            moved = true;
        }
    }
    else if (FALL_FUNC(key)) {
        while (updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0])){
            state->y++;
        }
        *fullFall = true;
        falledEvent(state, canvas);
        return;
    } else {
        state->leftKeyTime = 0;
        state->rightKeyTime = 0;
    }
    resetGhostBlock(canvas);
    renderGhostBlock(canvas, state);

    if (fallone){
        if (updateBlockPosition(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0])) {
            state->y++;
        }
        else if (!isKeyMovingPreventFall()) {
            falledEvent(state, canvas);
        }
        fallone = false;
    }
}

void resetGhostBlock(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]) {
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            if (!canvas[i][j].ghost) continue;
            resetBlock(&canvas[i][j]);
        }
    }
}

void renderGhostBlock(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
    int x = state->x;
    int y = state->y;
    int rotate = state->rotate;
    ShapeId shapeId = state->queue[0];
    Shape shapeData = shapes[shapeId];
    int size = shapeData.size;

    while (isPositionAvailable(canvas, x, y + 1, rotate, shapeData, size)) {
        y++;
    }

    if (isFallingBlockExist(canvas[y][x])) return;

    eraseGhostBlockPosition(canvas, x, y, size, rotate, shapeData);
    placeGhostBlockAtCoordinates(canvas, x, y, size, rotate, shapeData);
}

bool isOutOfCanvas(int x, int y) {
    return x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT;
}

bool isBlockExist(Block block) {
    return !block.current && (block.shape != EMPTY) && !block.ghost;
}

bool isFallingBlockExist(Block block) {
    return (block.shape != EMPTY) && !block.ghost;
}

bool isPositionAvailable(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int rotate, Shape shapeData, int size){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (!shapeData.rotates[rotate][i][j]) continue;

            Block that = canvas[y + i][x + j];
            if (isOutOfCanvas(x + j, y + i)) return false;
            if (isBlockExist(that)) return false;
        }
    }
    return true;
}

void eraseBlockPosition(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int size, int rotate, Shape shapeData){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (!shapeData.rotates[rotate][i][j]) continue;
            resetBlock(&canvas[y + i][x + j]);
        }
    }
}

void eraseGhostBlockPosition(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int size, int rotate, Shape shapeData){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (!shapeData.rotates[rotate][i][j]) continue;
            if (isFallingBlockExist(canvas[y + i][x + j])) continue;
            resetBlock(&canvas[y + i][x + j]);
        }
    }
}

void placeBlockAtCoordinates(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int size, int rotate, Shape shapeData){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (!shapeData.rotates[rotate][i][j]) continue;
            if (isBlockExist(canvas[y + i][x + j])) {
                gameOver = true;
                return;
            }
            setBlock(&canvas[y + i][x + j], shapeData.color, shapeData.shape, true, false);
        }
    }
}

void placeGhostBlockAtCoordinates(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int x, int y, int size, int rotate, Shape shapeData){
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (!shapeData.rotates[rotate][i][j]) continue;
            if (isFallingBlockExist(canvas[y + i][x + j])) continue;
            setBlock(&canvas[y + i][x + j], shapeData.color, shapeData.shape, true, true);
        }
    }
}

bool updateBlockPosition(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId) {
    Shape shapeData = shapes[shapeId];
    int size = shapeData.size;

    
    if (!isPositionAvailable(canvas, newX, newY, newRotate, shapeData, size)) return false;
    eraseBlockPosition(canvas, originalX, originalY, size, originalRotate, shapeData);
    placeBlockAtCoordinates(canvas, newX, newY, size, newRotate, shapeData);

    return true;

}

void pushUpBlocks(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int height){
    for (int i = 0; i < CANVAS_HEIGHT - height; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            canvas[i][j] = canvas[i + height][j];
        }
    }
    for (int i = CANVAS_HEIGHT - height; i < CANVAS_HEIGHT; i++) {
        int random = rand() % (CANVAS_WIDTH-1);
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            if (j == random) {
                resetBlock(&canvas[i][j]);
                continue;
            }
            else {
                canvas[i][j].color = WHITE;
                canvas[i][j].shape = O;
                canvas[i][j].current = false;
                canvas[i][j].ghost = false;
            }   
        }
    }
}

int clearLine(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH])
{
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].current)
            {
                canvas[i][j].current = false;
            }
        }
    }

    int linesCleared = 0;
    
    for (int i = CANVAS_HEIGHT - 1; i >= 0; i--)
    {
        bool isFull = true;
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].shape == EMPTY)
            {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            linesCleared += 1;

            for (int j = i; j > 0; j--)
            {
                for (int k = 0; k < CANVAS_WIDTH; k++)
                {
                    setBlock(&canvas[j][k], canvas[j - 1][k].color, canvas[j - 1][k].shape, false, false);
                    resetBlock(&canvas[j - 1][k]);
                }
            }
            i++;
        }
    }
    return linesCleared;
}