//
// Created by invik on 17/10/17.
//


#include "lxScrabble.h"

#include <csignal>

#include "dict_handler.h"
#include "scores_handler.h"
#include "game.h"

size_t wordlen;
u_long bonus;
string distrib;
string dict_file;
struct Cell *dictionary = nullptr;
struct Top topWeek[TOP_MAX];
struct Top topYear[TOP_MAX];
size_t foundWords;
size_t foundMaxWords;
size_t maxWordLen;
size_t dispMaxWords;
char dispMaxWordsString[1024];
run_state cur_state;
config cfgp;
string servername;
int port;
string server_pass;
string altnickname;
string ident;
string fullname;
string channelkey;
string perform;
u_int cfg_clock;
u_int cfg_warning;
u_int cfg_after;
u_int autostop;
bool autovoice;

void halt(int stat_code) {
    exit(stat_code);
}

template<class T> T cfg(const string & section, const string & option, const T & default_value) {
    if (! cfgp.contains(section))
        cfgp.add_section(section);
    if (! cfgp[section].contains(option))
        cfgp[section].add_option(option, (T) default_value);
    return cfgp[section][option].get<T>();
}

template<class T> vector<T> cfg_get_list(const string &section, const string &option, const T &default_value) {
    if (! cfgp.contains(section))
        cfgp.add_section(section);
    if (! cfgp[section].contains(option))
        cfgp[section].add_option(option, default_value);
    return cfgp[section][option].get_list<T>();
}

void readIni() {

    // Check that the config file exists
    if (! fexists(INI_FILE)) {
        cerr  << endl << endl << "Configuration file not found. Please put 'lxScrabble.ini' into this directory!" ;
        cerr << endl << endl;
        halt(1);
    }
    cfgp = parser::load_file(INI_FILE);

    // Game settings
    wordlen = cfg<size_t>("Settings", "wordlen", 12);
    bonus = cfg<unsigned_ini_t >("Settings", "bonus", 10);

    // Dictionary settings
    distrib = cfg<string>("Settings", "distribution",
                         "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOO"
                                 "PPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ");
    dict_file = cfg<string>("Settings", "dictionary", "english.dic");

    /* Read cfg with global reader */

    // Connection data
    servername = cfg<string>("IRC", "Server", DEFAULT_SERVER);
    if (strcmp(&servername[0], "<IRC SERVER HOSTNAME>") == 0) {
        cerr << endl << "Configure lxScrabble.ini first !!" << endl;
        halt(2);
    }
    port = (int) cfg<unsigned_ini_t>("IRC", "Port", DEFAULT_PORT);

    // IRC parameters
    bot_nick = cfg<string>("IRC", "Nick", DEFAULT_NICK);
    server_pass = cfg<string>("IRC", "Password", "");
    altnickname = cfg<string>("IRC", "ANick", DEFAULT_ANICK);
    ident = cfg<string>("IRC", "Ident", DEFAULT_IDENT);
    fullname = cfg<string>("IRC", "Fullname", BOTFULLNAME);

    // Channel
    channel = cfg<string>("IRC", "Channel", DEFAULT_CHANNEL);
    channelkey = cfg<string>("IRC", "ChannelKey", DEFAULT_CHANNEL_KEY);

    // Other configs
    irc_blackAndWhite = (bool) cfg<unsigned_ini_t>("IRC", "BlackAndWhite", 0);
    anyoneCanStop = (bool) cfg<unsigned_ini_t>("IRC", "AnyoneCanStop", 0);
    perform = cfg<string>("IRC", "Perform", "");
    owner = cfg_get_list<string>("IRC", "Owner", "");

    // Game parameters
    cfg_clock = (u_int) cfg<unsigned_ini_t>("Delay", "max", 40) * 10;
    cfg_warning = (u_int) cfg<unsigned_ini_t>("Delay", "warning", 30) * 10;
    cfg_after = (u_int) cfg<unsigned_ini_t>("Delay", "after", 30) * 10;
    autostop = (u_int) cfg<unsigned_ini_t>("Settings", "autostop", 3);
    autovoice = (bool) cfg<unsigned_ini_t>("Settings", "autovoice", 1);
}

void gentle_terminator(int) {
    cout << endl << "Terminating gently..." << endl;
    cur_state = HALTING;
}

void setup_interrupt_catcher() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = gentle_terminator;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);
}

int main(int argc, char *argv[]) {
    // Show banner, initialize random number generator
    cout << BOTFULLNAME << endl;
    srand((unsigned) time(nullptr));

    // Setup a handler to catch interrupt signals;
    setup_interrupt_catcher();

    // Read ini file
    readIni();

    // ReadDictionary
    readDictionary(dict_file);

    // Read top scores
    read_tops();

    // Connect and start game
    game_loop();

    return 0;
}

