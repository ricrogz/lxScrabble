//
// Created by invik on 17/10/17.
//

#include "lxScrabble.h"

#ifndef WSCRABBLE_SCORES_HANDLER_H
#define WSCRABBLE_SCORES_HANDLER_H

#define SCORE_FILE "scores.ini"
#define TOP_MAX 10


void clear_top(Top *top);

void read_top(Top *top, string & value);

void read_tops();


#endif //WSCRABBLE_SCORES_HANDLER_H
