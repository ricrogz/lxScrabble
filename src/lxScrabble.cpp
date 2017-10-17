//
// Created by invik on 17/10/17.
//


#include "lxScrabble.h"

#include "dict_handler.h"
#include "scores_handler.h"
#include "irc.h"



size_t wordlen;
long bonus;
string distrib;
string dict_file;
struct Cell *dictionary = nullptr;
struct Top topWeek[TOP_MAX];
struct Top topYear[TOP_MAX];



void halt(int stat_code) {
    exit(stat_code);
}

void strupr(string &str) {
    for (auto &c: str) c = (char) toupper(c);
}

void readIni() {

    INIReader reader(INI_FILE);

    // Check error
    int error_check = reader.ParseError();
    if (error_check < 0) {
        cout << "Can't load 'lxScrabble.ini'\n";
        halt(error_check);
    }

    // Game settings
    wordlen = (size_t) reader.GetInteger("Settings", "wordlen", 12);
    bonus = reader.GetInteger("Settings", "bonus", 10);

    // Dictionary settings
    distrib = reader.Get("Settings", "distribution",
                         "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOO"
                                 "PPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ");
    dict_file = reader.Get("Settings", "dictionary", "english.dic");
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

    // Connect to irc
    irc_connect();


#ifndef OFFLINE
#ifdef _WIN32
    // Cleanup winsocks
    WSACleanup();
#endif
#endif

    return 0;
}
