//
// Created by invik on 17/10/17.
//

#include "lxScrabble.h"

#ifndef WSCRABBLE_SCORES_HANDLER_H
#define WSCRABBLE_SCORES_HANDLER_H

#define SCORE_FILE "scores.ini"
#define TOP_MAX 10


void clear_top(Top *top);

void read_tops();

void get_scores(const string & nickname, u_long *year, u_long *week);

void set_scores(const string & nickname, u_long year, u_long week);

void clear_week_scores();

#endif //WSCRABBLE_SCORES_HANDLER_H
