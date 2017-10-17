//
// Created by invik on 17/10/17.
//

#include "neWScrabble.h"

#ifndef WSCRABBLE_SCORES_HANDLER_H
#define WSCRABBLE_SCORES_HANDLER_H

#define TOP_MAX 10


void clear_top(Top *top);

void read_top(Top *top, char *value);

void write_top(Top *top, char *value);

void read_tops();

void write_tops();

bool update_top(Top *top, const char *nickname, int score);

void clear_week_scores();

void get_scores(const char *nickname, int *year, int *week);

void set_scores(const char *nickname, int year, int week);


#endif //WSCRABBLE_SCORES_HANDLER_H
