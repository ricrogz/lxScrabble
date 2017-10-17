//
// Created by invik on 17/10/17.
//

#ifndef WSCRABBLE_NEWSCRABBLE_H
#define WSCRABBLE_NEWSCRABBLE_H


#define BOTNAME "WScrabble 1.10" // penser a mettre à jour VERSIONINFO aussi
#define BOTFULLNAME BOTNAME " by Wizou"
#define ADVERTISE "\x002""\x003""9,2 ~\x003""08  \x003""04¤ \x003""00" BOTNAME \
                  " \x003""15-\x003""00 http://wiz0u.free.fr/wscrabble \x003""04¤\x003""08  \x003""09~\x00F"""

#define TOP_MAX 10

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
extern struct Top topWeek[TOP_MAX];
extern struct Top topYear[TOP_MAX];

void halt(int stat_code);

void strupr(string &str);

#endif //WSCRABBLE_NEWSCRABBLE_H
