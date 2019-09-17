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
    Cell* other = nullptr;  // autres lettres possibles (plus grandes dans
                            // l'ordre alphab�tiques)
    Cell* longer = nullptr; // mots plus long disponibles

    // succession des mots contenant ces lettres, s�par�s par des
    // \0 et termin� par un autre \0
    std::vector<std::string> words;

    const char letter; // la lettre

    Cell() = delete;
    explicit Cell(const Cell&) = delete;
    Cell(char& l, Cell*&& o) noexcept : other(std::move(o)), letter(l) {}
    ~Cell()
    {
        delete other;
        delete longer;
    }

    void addWord(std::string&& w) { words.push_back(std::move(w)); }
    std::size_t size() const { return words.size(); }
    bool empty() const { return words.empty(); }
};

void addWord(Cell*& dictionary, std::string&& word);

Cell const* readDictionary(const std::string& filename);

struct FoundWords {
    std::size_t totalWords = 0;
    std::size_t lenBestWords = 0;
    std::vector<std::string> bestWords;
};

FoundWords findWords(Cell const*& dictionary, const std::string& letters);

#endif // LXSCRABBLE_DICT_HANDLER_H
