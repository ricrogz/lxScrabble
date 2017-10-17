//
// Created by invik on 17/10/17.
//

#include "lxScrabble.h"

#ifndef WSCRABBLE_DICT_HANDLER_H
#define WSCRABBLE_DICT_HANDLER_H

void sortLetters(const string &letters, char *sortedLetters);

void addWord(const string &word);

void readDictionary(const string &filename);


#endif //WSCRABBLE_DICT_HANDLER_H
