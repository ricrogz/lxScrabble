//
// Created by invik on 17/10/17.
//

#include "dict_handler.h"

#include <cstring>
#include <fstream>
#include <iostream>

void sortLetters(const string &letters, char *sortedLetters) {
    char *scan = sortedLetters;
    sortedLetters[0] = 127;
    for (char ch: letters) {
        scan = sortedLetters;
        while (ch >= *scan) scan++;
        char ch2;
        do {
            ch2 = *scan;
            *scan++ = ch;
        } while ((ch = ch2) != 127);
        *scan = 127;
    }
    *scan = 0;
}

void addWord(const string &word) {
    char letters[word.length() + 1];
    sortLetters(word, letters);
    struct Cell **cell = &dictionary;
    char *scan = letters;
    char ch = *scan;
    int len = 1;
    while (ch) {
        len++;
        for (;;) {
            if ((*cell == nullptr) || ((*cell)->letter > ch)) {
                auto *newcell = (struct Cell *) malloc(sizeof(struct Cell));
                newcell->other = *cell;
                newcell->longer = nullptr;
                newcell->words = nullptr;
                newcell->wordsCount = 0;
                newcell->letter = ch;
                *cell = newcell;
                break;
            } else if ((*cell)->letter == ch) break;
            cell = &(*cell)->other;
        }
        if ((ch = *++scan) != '\0')
            cell = &(*cell)->longer;
    }
    size_t wordsCount = (*cell)->wordsCount++;
    (*cell)->words = (char *) realloc((*cell)->words, (wordsCount + 1) * len);
    strcpy(&(*cell)->words[wordsCount * len], word.c_str());
}

void readDictionary(const string &filename) {
    ifstream stream(filename);
    if (stream.fail()) {
        cerr << "Could not open dictionnary file" << endl;
        halt(3);
    }
    string word;
    while (!stream.eof()) {
        stream >> word;
        if (0 == word.length()) continue;
        if (wordlen < word.length()) continue;
        strupr(word);

        if (strspn(word.c_str(), distrib.c_str()) != word.length()) {
            cerr << "Invalid dictionary entry: " << word << endl;
            cerr << "(contains lowercase letters or symbols not in the valid distribution)" << endl;
            halt(3);
        }

        addWord(word);
    }
}
