#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "getkey.h"

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 20
#define INITIAL_SIZE 5
#define FPS 10

#define HALT 0
#define UP 65
#define DOWN 66
#define LEFT 68
#define RIGHT 67
#define QUIT 'q'

#define APPLE 'O' 
#define SNAKE 'X'

#define OK 0
#define DEAD -1

struct position {
    int x;
    int y;
};

struct gameState {
    char board[BOARD_HEIGHT][BOARD_WIDTH];
    struct position player[BOARD_HEIGHT*BOARD_WIDTH];
    int playerIndex;
    struct position apple;
    int level;
    int direction;
    int error;
    int wrap;
};

static void renderBoard_(struct gameState *state);
static int normalizeIndex_(int index);
static void updateGame_(struct gameState *state);
static void resetPlayer_(struct gameState *state);
static void spawnApple_(struct gameState *state);
static void initializeGameState_(struct gameState *state);
static void resetBoard_(struct gameState *state);
static void printBoard_(struct gameState *state);
static void printBoardHorizontalBorder_(void);
static void handleInput_(struct gameState *state);

int main(int argc, char *argv[])
{
    struct gameState state;
    initializeGameState_(&state);
    state.wrap = (argc == 2);

    printf("\033[s");

    while(OK == state.error)
    {
        struct timespec wait = {0, (1000 * 1000 * 1000)/(FPS + state.level)};

        handleInput_(&state);
        updateGame_(&state);
        printBoard_(&state);
        nanosleep(&wait, NULL);
    }
    return 0;
}

static void handleInput_(struct gameState *state)
{
    int key = getkey();
    int nextKey = key;
    while (nextKey != -1)
    {
        nextKey = getkey();
        if (nextKey != -1)
        {
            key = nextKey;
        }
    }

    switch(key)
    {
        case UP:
        case DOWN:
        case LEFT:
        case RIGHT:
            state->direction = key;
            break;
        case QUIT:
            state->error = QUIT;
            break;
        case -1:
        default:
            break;
    }
}

static void initializeGameState_(struct gameState *state)
{
    resetPlayer_(state);
    spawnApple_(state);
    state->level = 1;
    state->direction = HALT; 
    renderBoard_(state);
    state->error = OK;
}

static void resetPlayer_(struct gameState *state)
{
    int i;
    for(i = 0; i < INITIAL_SIZE; ++i)
    {
        state->player[i].x = BOARD_WIDTH/2;
        state->player[i].y = BOARD_HEIGHT/2;
    }
    state->playerIndex = INITIAL_SIZE-1;
}

static void spawnApple_(struct gameState *state)
{
    state->apple.x = rand() % BOARD_WIDTH;
    state->apple.y = rand() % BOARD_HEIGHT;
}

static void resetBoard_(struct gameState *state)
{
    memset(state->board, ' ', sizeof(state->board));
}

static void renderBoard_(struct gameState *state)
{
    int i;
    struct position *head = &state->player[state->playerIndex];

    resetBoard_(state);

    state->board[state->apple.y][state->apple.x] = APPLE;

    for (i=0; i<INITIAL_SIZE + state->level - 1; ++i)
    {
        int index = normalizeIndex_(state->playerIndex-i);
        int tailX = state->player[index].x;
        int tailY = state->player[index].y;
        state->board[tailY][tailX] = SNAKE;

        if (i != 0 && tailX == head->x && tailY == head->y)
        {
            state->error = DEAD;
        }
    }
}

static void updateGame_(struct gameState *state)
{
    struct position *player = &state->player[state->playerIndex];
    int dx = 0;
    int dy = 0;

    switch (state->direction)
    {
        case UP:
            dy = -1;
            break;
        case DOWN:
            dy = 1;
            break;
        case LEFT:
            dx = -1;
            break;
        case RIGHT:
            dx = 1;
            break;
        case HALT:
        default:
            break;
    }

    if (dx || dy)
    {
        int nextIndex = state->playerIndex + 1;
        struct position *nextPlayer;

        nextIndex = normalizeIndex_(nextIndex);
        state->playerIndex = nextIndex;
        nextPlayer = &state->player[nextIndex]; 
        nextPlayer->x = player->x + dx;
        nextPlayer->y = player->y + dy;

        if (nextPlayer->x == state->apple.x && nextPlayer->y == state->apple.y)
        {
            ++state->level;
            spawnApple_(state);
        }
        else if(nextPlayer->x < 0 || nextPlayer->x >= BOARD_WIDTH ||
                nextPlayer->y < 0 || nextPlayer->y >= BOARD_HEIGHT)
        {
            if (state->wrap)
            {
                nextPlayer->x += BOARD_WIDTH;
                nextPlayer->x %= BOARD_WIDTH;
                nextPlayer->y += BOARD_HEIGHT;
                nextPlayer->y %= BOARD_HEIGHT;
            }
            else
            {
                state->error = DEAD;
            }
        }
        if (state->error == OK)
        {
            renderBoard_(state);
        }
    }
}

static int normalizeIndex_(int index)
{
    index += BOARD_HEIGHT*BOARD_WIDTH;
    index %= BOARD_HEIGHT*BOARD_WIDTH;
    return index;
}

static void printBoard_(struct gameState *state)
{
    int i;
    int j;

    (void)printf("Snake CLI - lvl %d\n", state->level);
    printBoardHorizontalBorder_();
    for (i=0; i<BOARD_HEIGHT; ++i)
    {
        (void)printf("|");
        for (j=0; j<BOARD_WIDTH; ++j)
        {
            (void)printf("%c", state->board[i][j]);
        }
        (void)printf("|\n");
    }
    printBoardHorizontalBorder_();
    if (OK == state->error)
    {
        printf("\033[u");
    }
}

static void printBoardHorizontalBorder_(void)
{
    int i;
    for (i = 0; i < (BOARD_WIDTH + 2); ++i)
    {
        (void)printf("=");
    }
    (void)printf("\n");
}

