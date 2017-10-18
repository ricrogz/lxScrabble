//
// Created by invik on 17/10/17.
//

#ifndef WSCRABBLE_NEWSCRABBLE_H
#define WSCRABBLE_NEWSCRABBLE_H

#define INI_FILE "lxScrabble.ini"
#define WORD_MAX 64

#include <cstddef>
#include <string>
#include <iostream>

#include "inicpp/inicpp.h"

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
    unsigned long score;
};

extern size_t wordlen;
extern long bonus;
extern string distrib;
extern string dict_file;
extern struct Cell *dictionary;
extern struct Top topWeek[];
extern struct Top topYear[];
extern int foundWords;
extern int foundMaxWords;
extern int maxWordLen;
extern int dispMaxWords;
extern char dispMaxWordsString[];

void halt(int stat_code);

void strupr(string &str);

char* strupr(char* s);

template<class T> T cfg(const string & section, const string & option, const T & default_value);

#endif //WSCRABBLE_NEWSCRABBLE_H
