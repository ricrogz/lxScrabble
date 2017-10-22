//
// Created by invik on 18/10/17.
//

#ifndef LXSCRABBLE_MIMICS_H
#define LXSCRABBLE_MIMICS_H

#define NONLATIN_LCASE "ñç"
#define NONLATIN_UCASE "ÑÇ"

#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>


inline bool is_valid_char(char c) {
    return  isascii(c) || strchr(NONLATIN_LCASE, c) || strchr(NONLATIN_UCASE, c);
}


inline char* strupr(char* s) {
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

inline void msleep(ulong t) {
    usleep((__useconds_t)t * 1000);
}

inline bool fexists (const std::string & name) {
    struct stat buffer = {0};
    return (stat (name.c_str(), &buffer) == 0);
}

#endif //LXSCRABBLE_MIMICS_H
