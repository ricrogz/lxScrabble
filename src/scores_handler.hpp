//
// Created by invik on 17/10/17.
//

#include "inicpp/inicpp.h"
#include <memory>

#ifndef LXSCRABBLE_SCORES_HANDLER_H
#define LXSCRABBLE_SCORES_HANDLER_H

struct Top {
    std::string nick;
    unsigned long score;
};

extern Top topWeek[];
extern Top topYear[];

extern std::unique_ptr<inicpp::config> scorep;

void clear_top(Top* top);

void read_tops();

void get_scores(const std::string& nickname, unsigned long* year,
                unsigned long* week);

void set_scores(const std::string& nickname, unsigned long year,
                unsigned long week);

void clear_week_scores();

#endif // LXSCRABBLE_SCORES_HANDLER_H
