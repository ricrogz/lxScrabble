//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_DICT_HANDLER_H
#define LXSCRABBLE_DICT_HANDLER_H

#include <memory>

#include "lxScrabble.hpp"

struct dict_stats {
    size_t too_long = 0;
    size_t wrong_symbols = 0;
    size_t loaded = 0;
    size_t total = 0;
};

class Cell
{
  public:
    static void addWord(Cell*& root_cell, std::string&& word);

    Cell() = delete;
    Cell(const Cell&) = delete;
    Cell(Cell&&) = delete;
    Cell& operator=(const Cell&) = delete;
    Cell& operator=(Cell&&) = delete;

    Cell(const char& l, Cell* o) noexcept;
    ~Cell();

    size_t size() const;
    bool empty() const;

    Cell* other = nullptr;  // autres lettres possibles (plus grandes dans
                            // l'ordre alphab�tiques)
    Cell* longer = nullptr; // mots plus long disponibles

    // succession des mots contenant ces lettres, s�par�s par des
    // \0 et termin� par un autre \0
    std::vector<std::string> words;

    const char letter; // la lettre

  private:
    void storeWord(std::string&& word);
};

std::unique_ptr<const Cell> readDictionary(const std::string& filename);

struct FoundWords {
    size_t totalWords = 0;
    size_t lenBestWords = 0;
    std::vector<std::string> bestWords;
};

FoundWords findWords(const Cell* const& dictionary, const std::string& letters);

#endif // LXSCRABBLE_DICT_HANDLER_H
