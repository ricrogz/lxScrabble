//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_IRC_H
#define LXSCRABBLE_IRC_H

#include "version.hpp"
#include <string>

const std::string IRC_BOLD = "\x002";
const std::string IRC_COLOR = "\x003";

const std::string BOT_URL = "https://github.com/ricrogz/lxScrabble";

const std::string BOTFULLNAME = BOTNAME + " by invik";
const std::string ADVERTISE =
    IRC_BOLD + IRC_COLOR + "9,2 ~" + IRC_COLOR + "8,2*" + IRC_COLOR + "9,2~ " +
    IRC_COLOR + "00" + BOTNAME + " " + IRC_COLOR + "15<+> " + IRC_COLOR + "00" +
    BOT_URL + IRC_COLOR + "9,2 ~" + IRC_COLOR + "8,2*" + IRC_COLOR + "9,2~ ";

const std::string DEFAULT_SERVER = "irc.chathispano.com";
const unsigned int DEFAULT_PORT = 6667;

const std::string DEFAULT_NICK = "Scrabblor";
const std::string DEFAULT_ANICK = "Scrabbl0r";
const std::string DEFAULT_IDENT = "lxScrabble";

const std::string DEFAULT_CHANNEL = "#scrabble";
const std::string DEFAULT_CHANNEL_KEY = "";
constexpr clock_t TIMEOUT = 60 * 1000; // 45 s

extern std::string servername;
extern int port;
extern std::string server_pass;
extern std::string altnickname;
extern std::string ident;
extern std::string fullname;
extern std::string channelkey;
extern std::string perform;

void irc_connect();

void irc_sendline(const std::string& line);

void irc_sendmsg(const std::string& dest);

void irc_sendnotice(const std::string& dest);

std::string irc_stripcodes(const std::string& text);

void irc_sendformat(bool set_endl, const std::string& lpKeyName,
                    const std::string& lpDefault, ...);

void irc_analyze(char* line, char** nickname, char** ident, char** hostname,
                 char** cmd, char** param1, char** param2, char** paramtext);

bool irc_recv(char line[]);

void irc_send(const std::string& text);

void irc_disconnect_msg(const std::string& msg);

void irc_disconnect();

#endif // LXSCRABBLE_IRC_H
