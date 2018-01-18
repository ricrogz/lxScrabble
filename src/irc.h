//
// Created by invik on 17/10/17.
//

#ifndef WSCRABBLE_IRC_H
#define WSCRABBLE_IRC_H

#include "lxScrabble.h"

#define IRC_BOLD "\x002"
#define IRC_COLOR "\x003"


#define BOT_URL "https://github.com/ricrogz/lxScrabble"
#define BOTFULLNAME BOTNAME" by invik"
#define ADVERTISE IRC_BOLD \
                    IRC_COLOR "9,2 ~" IRC_COLOR "8,2*" IRC_COLOR "9,2~ " \
                    IRC_COLOR "00" BOTNAME " " \
                    IRC_COLOR "15<+> " \
                    IRC_COLOR "00" BOT_URL \
                    IRC_COLOR "9,2 ~" IRC_COLOR "8,2*" IRC_COLOR "9,2~ " \

#define DEFAULT_SERVER "irc.chathispano.com"
#define DEFAULT_PORT 6667

#define DEFAULT_NICK "Scrabblor"
#define DEFAULT_ANICK "Scrabbl0r"
#define DEFAULT_IDENT "lxScrabble"

#define DEFAULT_CHANNEL "#scrabble"
#define DEFAULT_CHANNEL_KEY ""

#define CONNECT_TIMEOUT (45 * 1000) // 45 s


void irc_connect();

void irc_sendline(const string & line);

void irc_sendmsg(const string & dest);

void irc_sendnotice(const string & dest);

void irc_stripcodes(char *text);

void irc_sendformat(bool set_endl, const string & lpKeyName, const string & lpDefault, ...);

void irc_analyze(char *line, char **nickname, char **ident, char **hostname, char **cmd, char **param1, char **param2,
                 char **paramtext);

bool irc_recv(char line[]);

void irc_send(const string & text);

void irc_disconnect_msg(const string & msg);

void irc_disconnect();

#endif //WSCRABBLE_IRC_H
