//
// Created by invik on 17/10/17.
//

#include "dict_handler.h"

inline int compare_char(const void* a, const void* b) {
    return *(u_char*)a - *(u_char*)b;
}

inline void sortLetters(const char *letters, char *sortedLetters) {
    size_t len = strlen(letters);
    strncpy(sortedLetters, letters, len);
    qsort((void*)sortedLetters, len, sizeof(char), compare_char);
}

void addWord(const string &word) {

    // TODO: convert Cell to a class, and implement this as a method

    char letters[word.length() + 1];
    letters[word.length()] = '\0';
    sortLetters(&word[0], letters);
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
    strcpy(&(*cell)->words[wordsCount * len], &word[0]);
}

void readDictionary(const string &filename) {
    ifstream stream(filename);
    if (stream.fail()) {
        cerr << "Could not open dictionary file" << endl;
        halt(3);
    }
    string word;
    while (!stream.eof()) {
        getline(stream, word);

        // Remove whitespace at end
        char forbidden[] = " \r\n";
        size_t l = word.length();
        for (auto c : forbidden)
            while (*&word[l - 1] == c)
                *&word[l--] = '\0';

        // Skip if word is empty or too long
        if (0 == word.length()) continue;
        if (wordlen < word.length()) continue;
        strupr(&word[0]);

        if (strspn(&word[0], &distrib[0]) != word.length()) {
            cerr << "Invalid dictionary entry: " << word << endl;
            cerr << "(contains lowercase letters or symbols not in the valid distribution)" << endl;
            halt(3);
        }
        addWord(word);
    }
    stream.close();
}

void findWords(const struct Cell *cell, const char *letters, size_t len) {
    char ch = *letters++;
    while (cell && (cell->letter < ch)) cell = cell->other;
    if (cell) {
        if (cell->letter == ch) {
            if (cell->wordsCount) {
                if (dispMaxWords) {
                    if (len == dispMaxWords) {
                        for (size_t index = 0; index < cell->wordsCount; index++) {
                            strcat(dispMaxWordsString, " - ");
                            strcat(dispMaxWordsString, cell->words + (len + 1) * index);
                        }
                    }
                } else {
                    foundWords += cell->wordsCount;
                    if (len > maxWordLen) {
                        foundMaxWords = cell->wordsCount;
                        maxWordLen = len;
                    } else if (len == maxWordLen)
                        foundMaxWords += cell->wordsCount;
                }
            }
            if ((*letters != 0) && cell->longer) {
                findWords(cell->longer, letters, len + 1);
            }
            while (*letters == ch) letters++;
        }
        if (*letters != 0)
            findWords(cell, letters, len);
    }
}

void findWords(const char *letters) {
    dispMaxWords = 0;
    foundWords = 0;
    foundMaxWords = 0;
    maxWordLen = 0;
    findWords(dictionary, letters, 1);
}

void displayMaxWords(const char *letters, size_t len) {
    dispMaxWordsString[0] = '\0';
    dispMaxWords = len;
    foundWords = 0;
    foundMaxWords = 0;
    maxWordLen = 0;
    findWords(dictionary, letters, 1);
}
