//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_NEWSCRABBLE_H
#define LXSCRABBLE_NEWSCRABBLE_H

#define INI_FILE "lxScrabble.ini"

#include <cstdlib>
#include <cstddef>
#include <random>
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
    u_long score;
};

enum run_state {
    RUNNING, STOPPED, QUITTING
};

extern bool list_failed_words;
extern size_t wordlen;
extern u_long bonus;
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
extern string bot_nick;
extern string public_nick;
extern bool irc_blackAndWhite;
extern string channel;
extern vector<string> owner;
extern bool anyoneCanStop;
extern string servername;
extern int port;
extern string server_pass;
extern string altnickname;
extern string ident;
extern string fullname;
extern string channelkey;
extern string perform;
extern u_int cfg_clock;
extern u_int cfg_warning;
extern u_int cfg_after;
extern u_int autostop;
extern bool autovoice;
extern long reannounce;
extern minstd_rand *simple_rand;  // simple random generator

void halt(int stat_code);

template<class T> T cfg(const string & section, const string & option, const T & default_value);

#endif //LXSCRABBLE_NEWSCRABBLE_H
