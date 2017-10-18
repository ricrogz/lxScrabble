//
// Created by invik on 17/10/17.
//

#include "lxScrabble.h"

#ifndef WSCRABBLE_DICT_HANDLER_H
#define WSCRABBLE_DICT_HANDLER_H

void sortLetters(const char *letters, char *sortedLetters);

void addWord(const string &word);

void readDictionary(const string &filename);

void findWords(const char letters[]);

void displayMaxWords(const char letters[WORD_MAX], int len);

#endif //WSCRABBLE_DICT_HANDLER_H
