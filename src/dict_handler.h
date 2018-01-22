//
// Created by invik on 17/10/17.
//


#ifndef LXSCRABBLE_DICT_HANDLER_H
#define LXSCRABBLE_DICT_HANDLER_H

#include "lxScrabble.h"

struct dict_stats {
    size_t too_long = 0;
    size_t wrong_symbols = 0;
    size_t loaded= 0;
    size_t total = 0;
};

void sortLetters(const char *letters, char *sortedLetters);

void addWord(const string &word);

void readDictionary(const string &filename);

void findWords(const char *letters);

void displayMaxWords(const char *letters, size_t len);

#endif //LXSCRABBLE_DICT_HANDLER_H
