//
// Created by invik on 17/10/17.
//

#include "dict_handler.hpp"

void addWord(std::string&& word)
{
    std::string letters(word);
    std::sort(letters.begin(), letters.end());
    cellPtr* cell = &dictionary;
    auto scan = letters.begin();
    while (scan != letters.end()) {
        char ch = *scan;
        while (true) {
            if ((*cell == nullptr) || ((*cell)->letter > ch)) {
                *cell = new Cell(ch, std::move(*cell));
                break;
            } else if ((*cell)->letter == ch)
                break;
            cell = &(*cell)->other;
        }
        if (++scan != letters.end())
            cell = &(*cell)->longer;
    }
    (*cell)->addWord(std::move(word));
}

void readDictionary(const std::string& filename)
{
    std::ifstream stream(filename);
    if (stream.fail()) {
        log_stderr("Could not open dictionary file");
        halt(3);
    }
    std::string word;
    struct dict_stats words;
    while (!stream.eof()) {
        std::getline(stream, word);

        // Remove whitespace at end
        char forbidden[] = " \r\n";
        std::size_t l = word.length();
        for (auto c : forbidden)
            while (l && *&word[--l] == c)
                *&word[l] = '\0';

        // Skip if line is empty -- length recalculated after whitespace removal
        if (0 == word.length())
            continue;

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
        std::size_t valid_length;
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
        addWord(std::move(word));
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
        log_stderr("ERROR: No words were imported, game cannot continue.");
        halt(1);
    }
}

void findWords(cellPtr cell, const char* letters, std::size_t len)
{
    // Get next letter
    char ch = *letters++;

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
            if (dispMaxWords) {
                if (len == dispMaxWords) {
                    maxWordLen = len;
                    for (const auto& w : cell->words) {
                        strcat(dispMaxWordsString, " - ");
                        strcat(dispMaxWordsString, w.c_str());
                    }
                }
            } else {
                foundWords += cell->size();
                if (len > maxWordLen) {
                    foundMaxWords = cell->size();
                    maxWordLen = len;
                } else if (len == maxWordLen) {
                    foundMaxWords += cell->size();
                }
            }
        }
        if ((*letters != '\0') && cell->longer) {
            findWords(cell->longer, letters, len + 1);
        }
        while (*letters == ch)
            letters++;
    }
    if (*letters != '\0')
        findWords(cell, letters, len);
}

void findWords(const char* letters)
{
    dispMaxWords = 0;
    foundWords = 0;
    foundMaxWords = 0;
    maxWordLen = 0;
    findWords(dictionary, letters, 1);
}

void displayMaxWords(const char* letters, std::size_t len)
{
    dispMaxWordsString[0] = '\0';
    dispMaxWords = len;
    foundWords = 0;
    foundMaxWords = 0;
    maxWordLen = 0;
    findWords(dictionary, letters, 1);
}
