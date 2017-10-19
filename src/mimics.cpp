//
// Created by invik on 18/10/17.
//

#include "mimics.h"

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

bool kbhit() {
    /*
     * Taken from:
     * https://stackoverflow.com/questions/29335758/using-kbhit-and-getch-on-linux
     */

    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}

bool is_valid_char(char c) {
    return  isascii(c) || strchr(NONLATIN_LCASE, c) || strchr(NONLATIN_UCASE, c);
}

char* strupr(char* s) {
    char *pch;
    char *tmp = s;
    char lcases[] = NONLATIN_LCASE;
    for (;*tmp;++tmp) {
        if (isalpha(*tmp)) *tmp = (char)toupper(*tmp);
        else if ((pch = strchr(lcases, *tmp)) != nullptr) {
            size_t p = lcases - pch;
            *tmp = NONLATIN_UCASE[p];
        }
    }
    return s;
}

void msleep(ulong t) {
    usleep((__useconds_t)t * 1000);
}

bool fexists (const std::string & name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
