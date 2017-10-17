//
// Created by invik on 17/10/17.
//

#include "scores_handler.h"

#include <cstring>

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
    unsigned long colon_pos;
    while (start < value.length()) {
        colon_pos = value.find(':', start);
        top->nick = value.substr(start, colon_pos++ - start);
        start = value.find(' ', colon_pos);
        top->score = (unsigned long)strtol(value.substr(colon_pos, start++ - colon_pos).c_str(), nullptr, 10);
        top++;
    }
}
/*
void write_top(Top *top, const string & value) {
    for (int index = 0; index < TOP_MAX; index++) {
        if (top->score == 0) break;
        strcpy(value, top->nick);
        value += strlen(value);
        *value++ = ':';
        itoa(top->score, value, 10);
        value += strlen(value);
        *value++ = ' ';
        top++;
    }
    if (index) --value;
    *value = '\0';
}
*/
void read_tops() {

    string value;
    INIReader reader("scores.ini");

    // Weekly top
    value = reader.Get("Top", "Week", "");
    read_top(topWeek, value);

    // Yearly top
    value = reader.Get("Top", "Year", "");
    read_top(topYear, value);
}
/*
void write_tops() {
    char value[(NICK_MAX + 10) * TOP_MAX];
    write_top(topWeek, value);
    WritePrivateProfileString("Top", "Week", value, ".\\scores.ini");
    write_top(topYear, value);
    WritePrivateProfileString("Top", "Year", value, ".\\scores.ini");
}

bool update_top(Top *top, const char *nickname, int score) {
    int newPos;
    for (newPos = 0; newPos < TOP_MAX; newPos++)
        if (score >= top[newPos].score)
            break;
    if (newPos == TOP_MAX) return false; // le score n'entre pas dans le top
    for (int index = newPos; index < TOP_MAX - 1; index++)
        if (stricmp(top[index].nick, nickname) == 0)
            break; // le nom etait déjà dans le top
    // 0 1 2 3 4 5 6 7 8 9
    memmove(&top[newPos + 1], &top[newPos], (index - newPos) * sizeof(Top));
    strcpy(top[newPos].nick, nickname);
    top[newPos].score = score;
    return true;
}

void clear_week_scores() {
    char *buffer = (char *) malloc(32767);
    GetPrivateProfileSection("Scores", buffer, 32767, ".\\scores.ini");
    char *scan = buffer;
    while (*scan) {
        scan = strchr(scan, '=') + 1;
        scan = strchr(scan, ' ') + 1;
        *scan++ = '0';
        while (isdigit(*scan))
            *scan++ = ' ';
        scan = strchr(scan, '\0') + 1;
    }
    WritePrivateProfileSection("Scores", buffer, ".\\scores.ini");
    free(buffer);
    clear_top(topWeek);
    write_tops();
}

void get_scores(const char *nickname, int *year, int *week) {
    char value[50], *scan;
    char lnickname[NICK_MAX + 1];
    lnickname[0] = '_';
    strcpy(lnickname + 1, nickname);
    GetPrivateProfileString("Scores", lnickname, "0 0", value, sizeof(value), ".\\scores.ini");
    *year = strtoul(value, &scan, 10);
    *week = strtoul(scan, &scan, 10);
}

void set_scores(const char *nickname, int year, int week) {
    char value[50];
    char lnickname[NICK_MAX + 1];
    lnickname[0] = '_';
    strcpy(lnickname + 1, nickname);
    sprintf(value, "%d %d", year, week);
    WritePrivateProfileString("Scores", lnickname, value, ".\\scores.ini");
    bool updated = update_top(topWeek, nickname, week);
    updated |= update_top(topYear, nickname, year);
    if (updated) write_tops();
}
*/