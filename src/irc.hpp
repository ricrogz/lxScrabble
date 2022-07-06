//
// Created by invik on 17/10/17.
//

#ifndef LXSCRABBLE_IRC_H
#define LXSCRABBLE_IRC_H

#include <random>
#include <string>

#include "fmt/format.h"

#include "version.hpp"

const std::string IRC_BOLD = "\x002";
const std::string IRC_COLOR = "\x003";

const std::string BOT_URL = "ricrogz/lxScrabble @ github";

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
const std::string DEFAULT_CHANNEL_KEY;
constexpr clock_t PING_INTERVAL = 15 * 1000;
constexpr clock_t TIMEOUT = 90 * 1000; // default: 90 s

const size_t BUFFER_SIZE = 8192;
const size_t LINE_BUFFER_SIZE = 1024;

using RandGenerator = std::default_random_engine;

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

std::string get_fmt_template(const std::string& lpKeyName,
                             const std::string& lpDefault);

void irc_analyze(std::string&& line, std::string& nickname, std::string& cmd,
                 std::string& target, std::string& text);

bool irc_recv(std::string& line);

void irc_send(const std::string& text);

void irc_disconnect_msg(const std::string& msg);

void irc_disconnect();

template <typename... Args>
void irc_sendformat(bool set_endl, const std::string& lpKeyName,
                    const std::string& lpDefault, const Args&... args)
{
    auto buffer = get_fmt_template(lpKeyName, lpDefault);

    auto text = fmt::format(buffer, args...);
    if (set_endl) {
        text.push_back('\n');
    }

    irc_send(text);
}

class Pinger
{
  public:
    Pinger() = delete;
    Pinger(const Pinger&) = delete;
    Pinger(Pinger&&) = delete;
    Pinger& operator=(const Pinger&) = delete;
    Pinger& operator=(Pinger&&) = delete;
    ~Pinger() = default;

    explicit Pinger(RandGenerator& generator);

    void recv();
    bool is_alive();

  private:
    RandGenerator& d_generator;
    std::uniform_int_distribution<uint32_t> d_distrib;
    clock_t d_lastRecv;
    bool d_pinged = false;
};

#endif // LXSCRABBLE_IRC_H
