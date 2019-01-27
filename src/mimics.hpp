//
// Created by invik on 18/10/17.
//

#ifndef LXSCRABBLE_MIMICS_H
#define LXSCRABBLE_MIMICS_H

#include <climits>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

/*                               ñ    ç   0 */
const std::vector<char> NON_ASCII_LCASE({-15, -25});
const std::vector<char> NON_ASCII_UCASE({-47, -57});

inline bool is_valid_char(char c)
{
    return isascii(c) ||
           NON_ASCII_LCASE.end() !=
               std::find(NON_ASCII_LCASE.begin(), NON_ASCII_LCASE.end(), c) ||
           NON_ASCII_UCASE.end() !=
               std::find(NON_ASCII_UCASE.begin(), NON_ASCII_UCASE.end(), c);
}

inline char* non_ascii_strupr(char* s)
{
    char* tmp = s;
    auto pch = NON_ASCII_LCASE.end();
    for (; *tmp; ++tmp) {
        if (isalpha(*tmp))
            *tmp = toupper(*tmp);
        else if (NON_ASCII_LCASE.end() !=
                 (pch = std::find(NON_ASCII_LCASE.begin(),
                                  NON_ASCII_LCASE.end(), *tmp))) {
            *tmp = NON_ASCII_UCASE[pch - NON_ASCII_LCASE.begin()];
        }
    }
    return s;
}

inline void log(const char* message, std::ostream& stream)
{
    char timestamp[25];
    struct tm* sTm;
    time_t now = time(nullptr);
    sTm = localtime(&now);
    strftime(timestamp, 25, "%Y-%m-%d %H:%M:%S --- ", sTm);
    stream << timestamp << message << std::endl;
}

inline void log_stdout(const char* message)
{
    log(message, std::cout);
}

inline void log_stderr(const char* message)
{
    log(message, std::cerr);
}

inline void msleep(u_long t)
{
    usleep((__useconds_t) t * 1000);
}

inline bool fexists(const std::string& name)
{
    struct stat buffer;
    return stat(name.c_str(), &buffer) == 0;
}

#endif // LXSCRABBLE_MIMICS_H
