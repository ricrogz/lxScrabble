//
// Created by invik on 17/10/17.
//

#include "lxScrabble.hpp"

#ifndef LXSCRABBLE_SCORES_HANDLER_H
#define LXSCRABBLE_SCORES_HANDLER_H

#define SCORE_FILE "scores.ini"
#define TOP_MAX 10

struct Top {
    std::string nick;
    u_long score;
};

extern std::unique_ptr<inicpp::config> scorep;

void clear_top(Top* top);

void read_tops();

void get_scores(const std::string& nickname, u_long* year, u_long* week);

void set_scores(const std::string& nickname, u_long year, u_long week);

void clear_week_scores();

#endif // LXSCRABBLE_SCORES_HANDLER_H
