//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_NEWSCRABBLE_H
#define LXSCRABBLE_NEWSCRABBLE_H

#include <cstddef>
//#include <cstdlib>
#include <iostream>
#include <string>

#include "inicpp/inicpp.h"

#include "mimics.hpp"
#include "version.hpp"

struct Cell;
using cellPtr = struct Cell*;

enum run_state { RUNNING, STOPPED, QUITTING };

extern bool list_failed_words;
extern std::size_t wordlen;
extern u_long bonus;
extern std::string distrib;
extern std::string dict_file;
extern cellPtr dictionary;
extern struct Top topWeek[];
extern struct Top topYear[];
extern std::size_t foundWords;
extern std::size_t foundMaxWords;
extern std::size_t maxWordLen;
extern std::size_t dispMaxWords;
extern char dispMaxWordsString[];
extern run_state cur_state;
extern std::string bot_nick;
extern std::string public_nick;
extern bool irc_blackAndWhite;
extern std::string channel;
extern std::vector<std::string> owner;
extern bool anyoneCanStop;
extern std::string servername;
extern int port;
extern std::string server_pass;
extern std::string altnickname;
extern std::string ident;
extern std::string fullname;
extern std::string channelkey;
extern std::string perform;
extern u_int cfg_clock;
extern u_int cfg_warning;
extern u_int cfg_after;
extern u_int autostop;
extern bool autovoice;
extern long reannounce;

const std::string INI_FILE("lxScrabble.ini");

void halt(int stat_code);

template <class T>
T cfg(const std::string& section, const std::string& option,
      const T& default_value);

#endif // LXSCRABBLE_NEWSCRABBLE_H
