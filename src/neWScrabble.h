//
// Created by invik on 17/10/17.
//

#ifndef WSCRABBLE_NEWSCRABBLE_H
#define WSCRABBLE_NEWSCRABBLE_H

#include <cstddef>
#include <string>
#include <iostream>

#include "../lib/inih/cpp/INIReader.h"

using namespace std;

struct Cell {
    struct Cell *other; // autres lettres possibles (plus grandes dans l'ordre alphabétiques)
    struct Cell *longer; // mots plus long disponibles
    char *words; // succession des mots contenant ces lettres, séparés par des \0 et terminé par un autre \0
    size_t wordsCount;
    char letter; // la lettre
};

struct Top {
    string nick;
    unsigned long score;
};


extern size_t wordlen;
extern long bonus;
extern string distrib;
extern string dict_file;
extern struct Cell *dictionary;
extern struct Top topWeek[];
extern struct Top topYear[];

void halt(int stat_code);

void strupr(string &str);

#endif //WSCRABBLE_NEWSCRABBLE_H
