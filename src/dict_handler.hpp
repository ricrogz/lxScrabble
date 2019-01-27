//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_DICT_HANDLER_H
#define LXSCRABBLE_DICT_HANDLER_H

#include "lxScrabble.hpp"

struct dict_stats {
    std::size_t too_long = 0;
    std::size_t wrong_symbols = 0;
    std::size_t loaded = 0;
    std::size_t total = 0;
};

class Cell
{
  public:
    cellPtr other = nullptr;  // autres lettres possibles (plus grandes dans
                              // l'ordre alphab�tiques)
    cellPtr longer = nullptr; // mots plus long disponibles

    // succession des mots contenant ces lettres, s�par�s par des
    // \0 et termin� par un autre \0
    std::vector<std::string> words;

    const char letter = '0'; // la lettre

    Cell() = delete;
    Cell(const Cell&) = delete;
    Cell(char& l, cellPtr&& o) : other(std::move(o)), letter(l) {}
    ~Cell()
    {
        delete other;
        delete longer;
    }

    void addWord(std::string&& w) { words.push_back(std::move(w)); }
    std::size_t size() const { return words.size(); }
    bool empty() const { return words.empty(); }
};

void addWord(std::string&& word);

void readDictionary(const std::string& filename);

void findWords(const char* letters);

void displayMaxWords(const char* letters, std::size_t len);

#endif // LXSCRABBLE_DICT_HANDLER_H
