//
// Created by invik on 17/10/17.
//


#include "lxScrabble.h"

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
long bonus;
string distrib;
string dict_file;
struct Cell *dictionary = nullptr;
struct Top topWeek[TOP_MAX];
struct Top topYear[TOP_MAX];
int foundWords;
int foundMaxWords;
int maxWordLen;
int dispMaxWords;
char dispMaxWordsString[1024];

void halt(int stat_code) {
    exit(stat_code);
}

void strupr(string &str) {
    for (auto &c: str) c = (char) toupper(c);
}

char* strupr(char* s) {
    char* tmp = s;
    for (;*tmp;++tmp) {
        *tmp = (char)toupper(*tmp);
    }

    return s;
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

    // Game settings
    wordlen = cfg<size_t>("Settings", "wordlen", 12);
    bonus = cfg<unsigned_ini_t >("Settings", "bonus", 10);

    // Dictionary settings
    distrib = cfg<string>("Settings", "distribution",
                         "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOO"
                                 "PPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ");
    dict_file = cfg<string>("Settings", "dictionary", "english.dic");
}


int main(int argc, char *argv[]) {
    // Show banner, initialize random number generator
    cout << BOTFULLNAME << endl;
    srand((unsigned) time(nullptr));

    // Read ini file
    readIni();

    // ReadDictionary
    readDictionary(dict_file);

    // Read top scores
    read_tops();

#ifndef OFFLINE
#ifdef _WIN32
    // Initialize winsocks
    WSADATA wsaData;
    if ( WSAStartup(MAKEWORD( 2, 2 ), &wsaData) != 0 ) {
        cerr << "WSAStartup failed!" << endl;
        halt(1);
    }
#endif
#endif

    // Connect and start game
    game_loop();

#ifndef OFFLINE
#ifdef _WIN32
    // Cleanup winsocks
    WSACleanup();
#endif
#endif

    return 0;
}

