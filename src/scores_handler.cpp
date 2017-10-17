//
// Created by invik on 17/10/17.
//

#include "scores_handler.h"


void clear_top(Top *top) {
    for (int index = 0; index < TOP_MAX; index++) {
        top[index].score = 0;
        top[index].nick = "---";
    }
}

void read_top(Top *top, string & value) {
    clear_top(top);
    value.append(" ");
    unsigned long start = 0;
    while (start < value.length()) {
        unsigned long colon_pos = value.find(':', start);
        top->nick = value.substr(start, colon_pos++ - start);
        start = value.find(' ', colon_pos);
        top->score = (unsigned long)strtol(value.substr(colon_pos, start++ - colon_pos).c_str(), nullptr, 10);
        top++;
    }
}

void read_tops() {

    string value;
    INIReader reader("scores.ini");

    // Check error
    int error_check = reader.ParseError();
    if (error_check < 0) {
        cout << "Can't load 'scores.ini'\n";
        halt(error_check);
    }

    // Weekly top
    value = reader.Get("Top", "Week", "");
    read_top(topWeek, value);

    // Yearly top
    value = reader.Get("Top", "Year", "");
    read_top(topYear, value);
}
