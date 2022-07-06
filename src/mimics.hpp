//
// Created by invik on 18/10/17.
//

#ifndef LXSCRABBLE_MIMICS_H
#define LXSCRABBLE_MIMICS_H

#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

#include "fmt/chrono.h"

static const char* LOG_FMT = "{:%Y-%m-%d %H:%M:%S} --- {}\n";

const std::unordered_map<char, char> NON_ASCII_CONVERSIONS = {
    {241, 209}, // ñ >> Ñ
    {231, 199}, // ç >> Ç
};

inline bool is_valid_char(char& c)
{
    auto is_match = [&c](const std::pair<char, char>& p) {
        return c == p.first || c == p.second;
    };
    return (isascii(c) != 0) ||
           std::any_of(NON_ASCII_CONVERSIONS.begin(),
                       NON_ASCII_CONVERSIONS.end(), is_match);
}

inline char* non_ascii_strupr(char* s)
{
    for (char* tmp = s; *tmp != 0u; ++tmp) {
        if (isalpha(*tmp) != 0) {
            *tmp = toupper(*tmp);
        } else {
            auto found = NON_ASCII_CONVERSIONS.find(*tmp);
            if (found != NON_ASCII_CONVERSIONS.end()) {
                *tmp = found->second;
            }
        }
    }
    return s;
}

inline void log(const std::string& message)
{
    std::time_t now = std::time(nullptr);
    fmt::print(LOG_FMT, fmt::localtime(now), message);
}

inline void msleep(unsigned long t)
{
    usleep((__useconds_t) t * 1000);
}

inline bool fexists(const std::string& name)
{
    struct stat buffer;
    return stat(name.c_str(), &buffer) == 0;
}

#endif // LXSCRABBLE_MIMICS_H
