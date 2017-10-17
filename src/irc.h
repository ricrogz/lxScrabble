//
// Created by invik on 17/10/17.
//

#include "lxScrabble.h"

#ifndef WSCRABBLE_IRC_H
#define WSCRABBLE_IRC_H


#define BOTNAME "lxScrabble 0.10" // penser a mettre à jour VERSIONINFO aussi
#define BOTFULLNAME BOTNAME" by Invik"
#define ADVERTISE "\x002""\x003""9,2 ~\x003""08  \x003""04¤ \x003""00" BOTNAME \
                  " \x003""15-\x003""00 http://wiz0u.free.fr/wscrabble \x003""04¤\x003""08  \x003""09~\x00F"""

#define DEFAULT_SERVER "irc.chathispano.com"
#define DEFAULT_PORT 6667

#define DEFAULT_NICK "Scrabblor"
#define DEFAULT_ANICK "Scrabbl0r"
#define DEFAULT_IDENT "lxScrabble"

#define DEFAULT_CHANNEL "#scrabble"
#define DEFAULT_CHANNEL_KEY ""



void irc_connect();


#endif //WSCRABBLE_IRC_H
