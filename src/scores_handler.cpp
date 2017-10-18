//
// Created by invik on 17/10/17.
//

#include "scores_handler.h"

template<class T> vector<T> score_get_list(const string & section, const string & option, const T & default_value){
    static config scorep = parser::load_file(SCORE_FILE);
    if (! scorep.contains(section))
        scorep.add_section(section);
    if (! scorep[section].contains(option))
        scorep[section].add_option(option, default_value);
    return scorep[section][option].get_list<T>();
}

void clear_top(Top *top) {
    for (int index = 0; index < TOP_MAX; index++) {
        top[index].score = 0;
        top[index].nick = "---";
    }
}

void read_top(Top *top, vector<string> &value) {
    clear_top(top);
    for (auto & row : value) {
        ulong separator = row.find(' ');
        top->nick = row.substr(0, separator);
        top->score = (unsigned long) strtol(row.substr(separator + 1).c_str(), nullptr, 10);
        top++;
    }
}

void read_tops() {

    vector<string> value;

    // Weekly top
    value = score_get_list<string>("Top", "Week", "");
    read_top(topWeek, value);

    // Yearly top
    value = score_get_list<string>("Top", "Year", "");
    read_top(topYear, value);
}

void get_scores(const char *nickname, int *year, int *week) {
    /*
    char value[50], *scan;
    char lnickname[NICK_MAX + 1];
    lnickname[0] = '_';
    strcpy(lnickname + 1, nickname);
    GetPrivateProfileString("Scores", lnickname, "0 0", value, sizeof(value), ".\\scores.ini");
    *year = strtoul(value, &scan, 10);
    *week = strtoul(scan, &scan, 10);
    */
}

void set_scores(const char *nickname, int year, int week) {
    char value[50];
    /*
    char lnickname[NICK_MAX + 1];
    lnickname[0] = '_';
    strcpy(lnickname + 1, nickname);
    sprintf(value, "%d %d", year, week);
    WritePrivateProfileString("Scores", lnickname, value, ".\\scores.ini");
    bool updated = update_top(topWeek, nickname, week);
    updated |= update_top(topYear, nickname, year);
    if (updated) write_tops();
    */
}