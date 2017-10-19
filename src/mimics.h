//
// Created by invik on 18/10/17.
//

#ifndef LXSCRABBLE_MIMICS_H
#define LXSCRABBLE_MIMICS_H

#define NONLATIN_LCASE "ñç"
#define NONLATIN_UCASE "ÑÇ"


#include <string>

bool is_valid_char(char c);

void strupr(std::string &str);

char* strupr(char* s);

void msleep(ulong t);

bool fexists (const std::string & name);

#endif //LXSCRABBLE_MIMICS_H
