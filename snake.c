#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 20
#define INITIAL_SIZE 5
#define FPS 10

#define DOWN KEY_DOWN
#define UP KEY_UP
#define LEFT KEY_LEFT
#define RIGHT KEY_RIGHT
#define HALT (KEY_RIGHT+1)
#define QUIT 'q'

#define APPLE 'O' 
#define SNAKE 'X'

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

static void resetTerminal_(int ignore);
static void renderBoard_(struct gameState *state);
static void updateScore_(int level);
static int normalizeIndex_(int index);
static void updateGame_(struct gameState *state);
static void resetPlayer_(struct gameState *state);
static void spawnApple_(struct gameState *state);
static void initializeGameState_(struct gameState *state);
static void resetBoard_(struct gameState *state);
static void printBorder_(void);
static void printBoard_(struct gameState *state);
static void handleInput_(struct gameState *state);
static int isNewDirectionValid_(int oldDirection, int newDirection);

int main(int argc, char *argv[])
{
    struct gameState state;
    int terminalSize[2];

    (void)signal(SIGINT, resetTerminal_);

    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_GREEN, COLOR_GREEN);
    init_pair(3, COLOR_RED, COLOR_RED);

    getmaxyx(stdscr, terminalSize[0], terminalSize[1]);
    if (terminalSize[0] < (BOARD_HEIGHT+3))
    {
        endwin();
        fprintf(stderr, "Minimum terminal height of %d lines required.\n",
                                                                BOARD_HEIGHT+3);
        return 1;
    }
    if (terminalSize[1] < (BOARD_WIDTH+2))
    {
        endwin();
        fprintf(stderr, "Minimum terminal width of %d columns required.\n",
                                                                 BOARD_WIDTH+2);
        return 2;
    }

    initializeGameState_(&state);
    printBorder_();
    state.wrap = (argc == 2);
    refresh();

    while(OK == state.error)
    {
        struct timespec wait = {0, (1000 * 1000 * 1000)/(FPS + state.level)};

        handleInput_(&state);
        updateGame_(&state);
        printBoard_(&state);
        refresh();
        nanosleep(&wait, NULL);
    }
    getch();
    endwin();
    return 0;
}

static void resetTerminal_(int ignore)
{
    endwin();
    exit(3);
}

static void handleInput_(struct gameState *state)
{
    int key = getch();

    switch(key)
    {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            if (isNewDirectionValid_(state->direction, key))
            {
                state->direction = key;
            }
            break;
        case 'q':
            state->error = QUIT;
            break;
        case ERR:
        default:
            break;
    }
}

static void initializeGameState_(struct gameState *state)
{
    resetPlayer_(state);
    spawnApple_(state);
    state->level = 1;
    updateScore_(state->level);
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

static void updateScore_(int level)
{
    mvprintw(0, 0, "Snake CLI - lvl %-3d", level);
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
        case KEY_UP:
            dy = -1;
            break;
        case KEY_DOWN:
            dy = 1;
            break;
        case KEY_LEFT:
            dx = -1;
            break;
        case KEY_RIGHT:
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
            updateScore_(state->level);
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

static void printBorder_(void)
{
    attron(COLOR_PAIR(1));
    mvaddch(1, 0, ACS_ULCORNER);
    hline(ACS_HLINE, BOARD_WIDTH);
    mvaddch(1, BOARD_WIDTH+1, ACS_URCORNER);
    mvvline(2, 0, ACS_VLINE, BOARD_HEIGHT);
    mvvline(2, BOARD_WIDTH+1, ACS_VLINE, BOARD_HEIGHT);
    mvaddch(2+BOARD_HEIGHT, 0, ACS_LLCORNER);
    hline(ACS_HLINE, BOARD_WIDTH);
    mvaddch(2+BOARD_HEIGHT, BOARD_WIDTH+1, ACS_LRCORNER);
    attroff(COLOR_PAIR(1));
}

static void printBoard_(struct gameState *state)
{
    int i;
    int j;

    for (i=0; i<BOARD_HEIGHT; ++i)
    {
        move(i+2, 1);
        for (j=0; j<BOARD_WIDTH; ++j)
        {
            int color;
            char boardChar = state->board[i][j];
            switch (boardChar)
            {
                case APPLE:
                    color = 3;
                    break;
                case SNAKE:
                    color = 2;
                    break;
                default:
                    color = 1;
                    break;
            }
            attron(COLOR_PAIR(color));
            addch(boardChar);
            attroff(COLOR_PAIR(color));
        }
    }
}

static int isNewDirectionValid_(int oldDirection, int newDirection)
{
    int invalidDirections[] = {UP, DOWN, RIGHT, LEFT, HALT};
    int invalid = invalidDirections[(oldDirection - DOWN)];

    return (newDirection != invalid);
}
