//
// Created by invik on 17/10/17.
//

#ifndef WSCRABBLE_NEWSCRABBLE_H
#define WSCRABBLE_NEWSCRABBLE_H

#define INI_FILE "lxScrabble.ini"

#include <cstddef>
#include <string>
#include <iostream>

#include "inicpp/inicpp.h"

#include "version.h"
#include "mimics.h"

using namespace std;
using namespace inicpp;

struct Cell {
    struct Cell *other; // autres lettres possibles (plus grandes dans l'ordre alphabétiques)
    struct Cell *longer; // mots plus long disponibles
    char *words; // succession des mots contenant ces lettres, séparés par des \0 et terminé par un autre \0
    size_t wordsCount;
    char letter; // la lettre
};

struct Top {
    string nick;
    ulong score;
};

enum run_state {
    RUNNING, STOPPED, HALTING
};

extern size_t wordlen;
extern ulong bonus;
extern string distrib;
extern string dict_file;
extern struct Cell *dictionary;
extern struct Top topWeek[];
extern struct Top topYear[];
extern size_t foundWords;
extern size_t foundMaxWords;
extern size_t maxWordLen;
extern size_t dispMaxWords;
extern char dispMaxWordsString[];
extern run_state cur_state;

void halt(int stat_code);

template<class T> T cfg(const string & section, const string & option, const T & default_value);

#endif //WSCRABBLE_NEWSCRABBLE_H
