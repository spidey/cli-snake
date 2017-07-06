#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>

static struct termios orig;

static void resetTerminal_(void)
{
    int fd = fileno(stdin);
    tcsetattr(fd, TCSANOW, &orig);
}

//------------------------------------------------------------------------------
// getkey() returns the next char in the stdin buffer if available, otherwise
//          it returns -1 immediately.
//
int getkey(void)
{
    static int initialized_ = 0;
    int fd = fileno(stdin);
    int error;
    char ch;

    if (!initialized_)
    {
        struct termios attr;
        tcgetattr(fd, &attr);
        orig = attr;
        atexit(resetTerminal_);

        attr.c_lflag &= ~(ICANON | ECHO);
        attr.c_cc[VMIN] = 0;
        attr.c_cc[VTIME] = 0;

        tcsetattr(fd, TCSANOW, &attr);

        initialized_ = 1;
    }

    error=(read(fd, &ch, 1) != 1);

    return (error ? -1 : (int) ch);
}

