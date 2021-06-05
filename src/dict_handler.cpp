//
// Created by invik on 17/10/17.
//
#include <fstream>

#include "fmt/format.h"

#include "dict_handler.hpp"
#include "mimics.hpp"

Cell::Cell(const char& l, Cell* o) noexcept : other(o), letter(l) {}
Cell::~Cell()
{
    delete other;
    delete longer;
}

void Cell::addWord(Cell*& root_cell, std::string&& word)
{
    std::string letters(word);
    std::sort(letters.begin(), letters.end());
    Cell** cell = &root_cell;
    auto scan = letters.begin();
    while (scan != letters.end()) {
        char ch = *scan;
        while (true) {
            if (*cell == nullptr || (*cell)->letter > ch) {
                *cell = new Cell(ch, *cell);
                break;
            } else if ((*cell)->letter == ch) {
                break;
            }
            cell = &(*cell)->other;
        }
        if (++scan != letters.end()) {
            cell = &(*cell)->longer;
        }
    }
    (*cell)->storeWord(std::move(word));
}

size_t Cell::size() const
{
    return words.size();
}
bool Cell::empty() const
{
    return words.empty();
}

void Cell::storeWord(std::string&& word)
{
    words.push_back(std::move(word));
}

std::unique_ptr<const Cell> readDictionary(const std::string& filename)
{
    const std::string whitespace_chars{" \r\n"};

    std::ifstream stream(filename);
    if (stream.fail()) {
        throw std::runtime_error("Could not open dictionary file");
    }

    // Make sure that at least the root cell is not nullptr.
    // Also, manage it from here on.
    auto root_cell = new Cell(distrib[0], nullptr);
    std::unique_ptr<const Cell> dictionary{root_cell};

    std::string word;
    struct dict_stats words;
    while (!stream.eof()) {
        std::getline(stream, word);

        // Remove whitespace at end
        size_t l = word.length();
        for (const auto& c : whitespace_chars)
            while (l && *&word[--l] == c) {
                *&word[l] = '\0';
            }

        // Skip if line is empty -- length recalculated after whitespace removal
        if (word.empty()) {
            continue;
        }

        ++words.total;

        // Skip too long words
        if (wordlen < word.length()) {
            if (list_failed_words) {
                log(fmt::format(
                    " -- word is too long for configured word length "
                    "({}): {}",
                    wordlen, word));
            }
            ++words.too_long;
            continue;
        }

        // Make uppercase and check for invalid characters
        non_ascii_strupr(&word[0]);
        size_t valid_length = 0;
        if ((valid_length = strspn(word.c_str(), distrib.c_str())) !=
            word.length()) {
            if (list_failed_words) {
                log(fmt::format(
                    " -- word contains non allowed character '{}': {}",
                    word.at(valid_length), word));
            }
            ++words.wrong_symbols;
            continue;
        }
        Cell::addWord(root_cell, std::move(word));
        ++words.loaded;
    }
    stream.close();

    // Report imported words
    log(fmt::format("Read {} words:", words.total));
    log(fmt::format(" {} were skipped for being longer than {} chars.",
                    words.too_long, wordlen));
    log(fmt::format(" {} were skipped because of disallowed chars.",
                    words.wrong_symbols));
    log(fmt::format("Using {} valid words.", words.loaded));
    if (!list_failed_words) {
        log("To print the list of skipped words, rerun with --list "
            "parameter.");
    }

    // Exit if no words could be imported
    if (words.loaded == 0) {
        throw std::runtime_error(
            "ERROR: No words were imported, game cannot continue.");
    }

    return dictionary;
}

void findWords(FoundWords& found, Cell const* cell, const std::string& letters,
               size_t len)
{
    // Get next letter
    auto pos = letters.begin() + len;
    char ch = *pos++;

    // Advance to the cell matching current letter
    while (cell && cell->letter < ch) {
        cell = cell->other;
    }

    // If no cell, exit
    if (!cell) {
        return;
    }

    // Explore cell
    if (cell->letter == ch) {
        if (!cell->empty()) {
            size_t word_len = cell->words[0].size();
            found.totalWords += cell->size();
            if (word_len > found.lenBestWords) {
                found.lenBestWords = word_len;
                found.bestWords.assign(cell->words.begin(), cell->words.end());
            } else if (word_len == found.lenBestWords) {
                found.bestWords.insert(found.bestWords.end(),
                                       cell->words.begin(), cell->words.end());
            }
        }

        if (pos != letters.end() && cell->longer) {
            findWords(found, cell->longer, letters, pos - letters.begin());
        }
        while (pos != letters.end() && *pos == ch) {
            ++pos;
        }
    }
    if (pos != letters.end()) {
        findWords(found, cell, letters, pos - letters.begin());
    }
}

FoundWords findWords(const Cell* const& dictionary, const std::string& letters)
{
    FoundWords found;
    findWords(found, dictionary, letters, 0);
    return found;
}
