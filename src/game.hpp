//
// Created by invik on 17/10/17.
//

#include "irc.hpp"
#include "lxScrabble.hpp"

#ifndef LXSCRABBLE_GAME_H
#define LXSCRABBLE_GAME_H

enum run_state { RUNNING, STOPPED, QUITTING };

extern run_state cur_state;

void game_loop(Cell const* dictionary);

#endif // LXSCRABBLE_GAME_H
