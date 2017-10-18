//
// Created by invik on 18/10/17.
//

#include "mimics.h"

#include <sys/ioctl.h>
#include <termios.h>
#include <cstring>


bool kbhit() {
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

void strupr(std::string &str) {
    for (auto &c: str) c = (char) toupper(c);
}

char* strupr(char* s) {
    char* tmp = s;
    for (;*tmp;++tmp) {
        *tmp = (char)toupper(*tmp);
    }

    return s;
}

