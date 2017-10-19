//
// Created by invik on 18/10/17.
//

#include "mimics.h"

#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

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
