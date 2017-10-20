//
// Created by invik on 17/10/17.
//


#include "lxScrabble.h"

#include <signal.h>

#include "dict_handler.h"
#include "scores_handler.h"
#include "game.h"

/*
 * TODO: cambiar de ini parser. inih no escribe.
 *
 * Probar inicpp https://github.com/SemaiCZE/inicpp
 *
 */


size_t wordlen;
ulong bonus;
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

void halt(int stat_code) {
    exit(stat_code);
}

template<class T> T cfg(const string & section, const string & option, const T & default_value) {
    static config cfgp = parser::load_file(INI_FILE);
    if (! cfgp.contains(section))
        cfgp.add_section(section);
    if (! cfgp[section].contains(option))
        cfgp[section].add_option(option, (T) default_value);
    return cfgp[section][option].get<T>();
}

void readIni() {

    // Check that the config file exists
    if (! fexists(INI_FILE)) {
        cerr  << endl << endl << "Configuration file not found. Please put 'lxScrabble.ini' into this directory!" ;
        cerr << endl << endl;
        halt(1);
    }

    // Game settings
    wordlen = cfg<size_t>("Settings", "wordlen", 12);
    bonus = cfg<unsigned_ini_t >("Settings", "bonus", 10);

    // Dictionary settings
    distrib = cfg<string>("Settings", "distribution",
                         "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOO"
                                 "PPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ");
    dict_file = cfg<string>("Settings", "dictionary", "english.dic");
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

