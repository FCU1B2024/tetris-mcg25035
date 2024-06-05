#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include "game.h"
#include "shapes.h"
#include "tcp_linux.h"

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

int key;
bool fallone = false;
int alertHeight = 6;

bool test = true;

State state;

void init_colors() {
    start_color();
    init_pair(RED - 40, COLOR_BLACK, COLOR_RED);
    init_pair(GREEN - 40, COLOR_BLACK, COLOR_GREEN);
    init_pair(YELLOW - 40, COLOR_BLACK, COLOR_YELLOW);
    init_pair(BLUE - 40, COLOR_BLACK, COLOR_BLUE);
    init_pair(PURPLE - 40, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(CYAN - 40, COLOR_BLACK, COLOR_CYAN);
    init_pair(WHITE - 40, COLOR_BLACK, COLOR_WHITE);
    init_pair(BLACK, COLOR_BLACK, COLOR_BLACK); // For completeness
}

void ncurse_init(){
    initscr();          // 初始化 ncurses
    cbreak();           // 禁止行缓冲
    noecho();           // 不显示输入
    nodelay(stdscr, TRUE); // 非阻塞输入
    keypad(stdscr, TRUE);
    init_colors();
}

void output_ghost_block(Color color) {
    addstr("=="); // Output two spaces
}

void output_color_block(Color color) {
    attron(COLOR_PAIR(color - 40));
    addstr("  "); // Output two spaces
    attroff(COLOR_PAIR(color - 40));
}

void output_empty_block() {
    addstr("  "); // Output two spaces
}

int findSubstringPosition(const char *str, const char *substr) {
    char *pos = strstr(str, substr); // Use strstr to find the substring

    if (pos != NULL) {
        return pos - str; // Calculate and return the position
    } else {
        return -1; // Return -1 if the substring is not found
    }
}

void printCanvas(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], Block canvas2[CANVAS_HEIGHT][CANVAS_WIDTH], State* state) {
    clear(); // Clear screen
    move(0, 0);

    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        addch('|');
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            if (canvas[i][j].ghost) {
                output_ghost_block(canvas[i][j].color);
            }
            else{
                output_color_block(canvas[i][j].color);
            }
        }
        if (CANVAS_HEIGHT-i < state->damage+1){
            attron(COLOR_PAIR(RED - 40));
        }
        addch('|');
        if (CANVAS_HEIGHT-i < state->damage+1){
            attroff(COLOR_PAIR(RED - 40));
        }
        addstr("                       ");
        addch('|');
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            if (canvas2[i][j].ghost) {
                output_ghost_block(canvas2[i][j].color);
            }
            else{
                output_color_block(canvas2[i][j].color);
            }
        }
        addch('|');
        addch('\n'); // Newline at the end of the row
    }

    // Output "Next:" label
    move(3, CANVAS_WIDTH * 2 + 5);
    addstr("Next:");

    // Output upcoming blocks
    for (int i = 1; i <= 3; i++) {
        Shape shapeData = shapes[state->queue[i]];
        for (int j = 0; j < 4; j++) {
            move(i * 4 + j, CANVAS_WIDTH * 2 + 6);
            for (int k = 0; k < 4; k++) {
                if (j < shapeData.size && k < shapeData.size && shapeData.rotates[0][j][k]) {
                    output_color_block(shapeData.color);
                } else {
                    output_empty_block();
                }
            }
        }
    }

    addstr("Score: ");
    printw("%d", state->score);
    addstr(" Damage: ");
    printw("%d", state->damage);


    refresh(); // Refresh the screen to show all updates
}

void syncCanvas(char* buffer, Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH]){
    int i = findSubstringPosition(buffer, "st");
    if (i == -1) return;
    i += 2;
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            if (buffer[i] == '*') {
                i++;
            }
            if (buffer[i] == '0') {
                canvas[y][x] = (Block){
                    .color = BLACK,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'R'){
                canvas[y][x] = (Block){
                    .color = RED,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'G'){
                canvas[y][x] = (Block){
                    .color = GREEN,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'Y'){
                canvas[y][x] = (Block){
                    .color = YELLOW,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'B'){
                canvas[y][x] = (Block){
                    .color = BLUE,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'P'){
                canvas[y][x] = (Block){
                    .color = PURPLE,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'C'){
                canvas[y][x] = (Block){
                    .color = CYAN,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            else if (buffer[i] == 'W'){
                canvas[y][x] = (Block){
                    .color = WHITE,
                    .shape = O,
                    .ghost = false,
                    .current = false
                };
            }
            i++;
        }
    }
}

void getDataFromServer(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state){
    char buffer[1024];
    read_from_server(buffer);
    syncCanvas(buffer, canvas);
    char damageStr[10];
    int start = findSubstringPosition(buffer, "damage.data");
    if (start == -1) return;
    start += 11;
    int indx = 0;
    while (buffer[start + indx] != 'd'){
        damageStr[indx] = buffer[start + indx];
        indx++;
    }

    state->damage += atoi(damageStr);
}

int main() {
    if (!init()) return 0;
    ncurse_init();
    srand(time(NULL));

    state = (State){
        .x = CANVAS_WIDTH / 2,
        .y = 0,
        .score = 0,
        .rotate = 0,
        .fallTime = 0,
        .leftKeyTime = 0,
        .rightKeyTime = 0,
        .damage = 0
    };

    for (int i = 0; i < 4; i++) {
        state.queue[i] = rand() % 7;
    }

    Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
    Block canvasB[CANVAS_HEIGHT][CANVAS_WIDTH];
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            resetBlock(&canvas[i][j]);
        }
    }

    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            resetBlock(&canvasB[i][j]);
        }
    }

    Shape shapeData = shapes[state.queue[0]];

    for (int i = 0; i < shapeData.size; i++) {
        for (int j = 0; j < shapeData.size; j++) {
            if (shapeData.rotates[0][i][j]) {
                setBlock(&canvas[state.y + i][state.x + j], shapeData.color, state.queue[0], true, false);
            }
        }
    }

    int sleep_interval = 10000; // 10 milliseconds
    int fall_interval = 300000; // 0.3 seconds
    int fall_timer = 0;
    int send_interval = 200000; // 0.5 second
    int send_timer = 0;
    bool full_fall = false;
    bool comboContinue = false;

    while (1) {
        getDataFromServer(canvasB, &state);
        int elapsedTime = sleep_interval; // 每次循环的时间间隔
        printCanvas(canvas, canvasB, &state);
        logic(canvas, &state, elapsedTime, &full_fall);
        usleep(sleep_interval);
        alertHeight = state.damage;

        fall_timer += sleep_interval;
        send_timer += sleep_interval;

        if (send_timer >= send_interval) {
            send_timer = 0;
            sendCanvasToServer(canvas);
        }

        if (fall_timer >= fall_interval) {
            fallone = true;
            fall_timer = 0;
        }

        if (full_fall) {
            full_fall = false;
            fallone = true;
            fall_timer = 0;   
        }

        if (comboContinue) {
            state.score = 0;
            comboContinue = false;
        }
    }

    endwin(); // 恢复正常终端行为
    return 0;
}
