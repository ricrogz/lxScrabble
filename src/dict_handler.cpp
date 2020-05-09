//
// Created by invik on 17/10/17.
//
#include <cstring>
#include <fstream>
#include <sstream>

#include "dict_handler.hpp"
#include "mimics.hpp"

void addWord(Cell*& dictionary, std::string&& word)
{
    std::string letters(word);
    std::sort(letters.begin(), letters.end());
    Cell** cell = &dictionary;
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
    (*cell)->addWord(std::move(word));
}

std::unique_ptr<const Cell> readDictionary(const std::string& filename)
{
    const std::string whitespace_chars{" \r\n"};
    Cell* dictionary = nullptr;

    std::ifstream stream(filename);
    if (stream.fail()) {
        throw std::runtime_error("Could not open dictionary file");
    }
    std::string word;
    struct dict_stats words;
    while (!stream.eof()) {
        std::getline(stream, word);

        // Remove whitespace at end
        std::size_t l = word.length();
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
                std::stringstream ss;
                ss << word << " -- word is too long for configured wordlen ("
                   << wordlen << ')';
                log_stdout(ss.str().c_str());
            }
            ++words.too_long;
            continue;
        }

        // Make uppercase and check for invalid characters
        non_ascii_strupr(&word[0]);
        std::size_t valid_length = 0;
        if ((valid_length = strspn(word.c_str(), distrib.c_str())) !=
            word.length()) {
            if (list_failed_words) {
                std::stringstream ss;
                ss << word << " -- word contains invalid character '"
                   << word.at(valid_length) << '\'';
                log_stdout(ss.str().c_str());
            }
            ++words.wrong_symbols;
            continue;
        }
        addWord(dictionary, std::move(word));
        ++words.loaded;
    }
    stream.close();

    // Report imported words
    std::stringstream ss;
    ss << "Read " << words.total << " words:" << std::endl;
    ss << "  " << words.too_long << " too long (> " << wordlen << " letters)."
       << std::endl;
    ss << "  " << words.wrong_symbols << " with invalid symbols." << std::endl;
    ss << "  " << words.loaded << " valid words." << std::endl;
    if (!list_failed_words) {
        ss << "To list invalid words, execute with --list parameter."
           << std::endl;
    }
    ss << std::endl;
    log_stdout(ss.str().c_str());

    // Exit if no words could be imported
    if (words.loaded == 0) {
        throw std::runtime_error(
            "ERROR: No words were imported, game cannot continue.");
    }

    return std::unique_ptr<const Cell>{dictionary};
}

void findWords(FoundWords& found, Cell const* cell, const std::string& letters,
               std::size_t len)
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
            std::size_t word_len = cell->words[0].size();
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

FoundWords findWords(Cell const*& dictionary, const std::string& letters)
{
    FoundWords found;
    findWords(found, dictionary, letters, 0);
    return found;
}
