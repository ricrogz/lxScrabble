//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_NEWSCRABBLE_H
#define LXSCRABBLE_NEWSCRABBLE_H

#include <cstddef>
#include <string>
#include <vector>

extern bool list_failed_words;
extern size_t wordlen;
extern unsigned long bonus;
extern std::string distrib;
extern std::string dict_file;
extern std::string bot_nick;
extern std::string public_nick;
extern bool irc_blackAndWhite;
extern std::string channel;
extern std::vector<std::string> owner;
extern bool anyoneCanStop;
extern unsigned int cfg_clock;
extern unsigned int cfg_warning;
extern unsigned int cfg_after;
extern unsigned int autostop;
extern bool autovoice;
extern long reannounce;

template <class T>
T cfg(const std::string& section, const std::string& option,
      const T& default_value);

#endif // LXSCRABBLE_NEWSCRABBLE_H
