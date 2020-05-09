//
// Created by invik on 17/10/17.
//

#include <csignal>
#include <memory>

#include "dict_handler.hpp"
#include "game.hpp"
#include "inicpp/inicpp.h"
#include "irc.hpp"
#include "lxScrabble.hpp"
#include "mimics.hpp"
#include "scoreboard.hpp"

bool list_failed_words = false;
std::size_t wordlen;
unsigned long bonus;
std::string distrib;
std::string dict_file;
std::unique_ptr<inicpp::config> cfgp;
Scoreboard* scores;
unsigned int cfg_clock;
unsigned int cfg_warning;
unsigned int cfg_after;
unsigned int autostop;
bool autovoice;
long reannounce;

const std::string INI_FILE("lxScrabble.ini");
const std::string SCORE_FILE = "scores.ini";
const std::size_t TOP_MAX = 10;

void halt(int stat_code)
{
    exit(stat_code);
}

template <class T>
T cfg(const std::string& section, const std::string& option,
      const T& default_value)
{
    if (!cfgp->contains(section))
        cfgp->add_section(section);
    if (!(*cfgp)[section].contains(option))
        (*cfgp)[section].add_option(option, (T) default_value);
    return (*cfgp)[section][option].get<T>();
}

template <class T>
std::vector<T> cfg_get_list(const std::string& section,
                            const std::string& option, const T& default_value)
{
    if (!cfgp->contains(section))
        cfgp->add_section(section);
    if (!(*cfgp)[section].contains(option))
        (*cfgp)[section].add_option(option, default_value);
    return (*cfgp)[section][option].get_list<T>();
}

std::string strip_passwd(const std::string& nick)
{

    auto split1 = std::find(nick.begin(), nick.end(), ':');
    auto split2 = std::find(nick.begin(), split1, '!');

    return std::string(nick.begin(), split2);
}

void readIni()
{

    // Check that the config file exists
    if (!fexists(INI_FILE)) {
        log_stderr("\n\nConfiguration file not found. Please put "
                   "'lxScrabble.ini' into this directory!\n\n");
        halt(1);
    }
    cfgp = std::make_unique<inicpp::config>(
        std::move(inicpp::parser::load_file(INI_FILE)));

    // Game settings
    wordlen = cfg<inicpp::unsigned_ini_t>("Settings", "wordlen", 12);
    bonus = cfg<inicpp::unsigned_ini_t>("Settings", "bonus", 10);

    // Dictionary settings
    distrib = cfg<std::string>(
        "Settings", "distribution",
        "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOO"
        "PPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ");
    std::sort(distrib.begin(), distrib.end());

    dict_file = cfg<std::string>("Settings", "dictionary", "english.dic");

    /* Read cfg with global reader */

    // Connection data
    servername = cfg<std::string>("IRC", "Server", DEFAULT_SERVER);
    if (servername == "<IRC SERVER HOSTNAME>") {
        log_stderr("\nConfigure lxScrabble.ini first !!");
        halt(2);
    }
    port = cfg<inicpp::unsigned_ini_t>("IRC", "Port", DEFAULT_PORT);

    // IRC parameters
    bot_nick = cfg<std::string>("IRC", "Nick", DEFAULT_NICK);
    server_pass = cfg<std::string>("IRC", "Password", "");
    altnickname = cfg<std::string>("IRC", "ANick", DEFAULT_ANICK);
    ident = cfg<std::string>("IRC", "Ident", DEFAULT_IDENT);
    fullname = cfg<std::string>("IRC", "Fullname", BOTFULLNAME);

    public_nick = strip_passwd(bot_nick);

    // Channel
    channel = cfg<std::string>("IRC", "Channel", DEFAULT_CHANNEL);
    channelkey = cfg<std::string>("IRC", "ChannelKey", DEFAULT_CHANNEL_KEY);

    // Other configs
    irc_blackAndWhite = cfg<inicpp::unsigned_ini_t>("IRC", "BlackAndWhite", 0);
    anyoneCanStop = cfg<inicpp::unsigned_ini_t>("IRC", "AnyoneCanStop", 0);
    perform = cfg<std::string>("IRC", "Perform", "");
    owner = cfg_get_list<std::string>("IRC", "Owner", "");

    // Game parameters
    cfg_clock = cfg<inicpp::unsigned_ini_t>("Delay", "max", 40) * 10;
    cfg_warning = cfg<inicpp::unsigned_ini_t>("Delay", "warning", 30) * 10;
    cfg_after = cfg<inicpp::unsigned_ini_t>("Delay", "after", 30) * 10;
    autostop = cfg<inicpp::unsigned_ini_t>("Settings", "autostop", 3);
    autovoice = cfg<inicpp::unsigned_ini_t>("Settings", "autovoice", 1);
    reannounce = cfg<inicpp::unsigned_ini_t>("Delay", "reannounce", 300);
}

void gentle_terminator(int)
{
    log_stdout("");
    log_stdout("Terminating gently...");
    cur_state = QUITTING;
}

void setup_interrupt_catcher()
{
    struct sigaction sigTermHandler;

    sigTermHandler.sa_handler = gentle_terminator;
    sigemptyset(&sigTermHandler.sa_mask);
    sigTermHandler.sa_flags = 0;

    sigaction(SIGTERM, &sigTermHandler, nullptr);
}

int main(int argc, char* argv[])
{
    // Show banner, initialize random number generator
    log_stdout(BOTFULLNAME);

    // Detect --list parameter
    for (int i = 0; i < argc; ++i) {
        list_failed_words = strcmp("--list", argv[i]) == 0;
    }

    // Setup a handler to catch interrupt signals;
    setup_interrupt_catcher();

    // Read ini file
    readIni();

    // ReadDictionary
    auto dictionary = readDictionary(dict_file);

    // Read top scores
    scores = Scoreboard::read_scoreboard(SCORE_FILE);

    // Connect and start game
    game_loop(dictionary.get());

    return 0;
}
