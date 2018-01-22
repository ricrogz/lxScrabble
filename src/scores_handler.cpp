//
// Created by invik on 17/10/17.
//

#include "scores_handler.h"

template<class T> void score_add(const string &section, const string &option, const T &value) {
    if (!scorep->contains(section))
        scorep->add_section(section);
    if (! (*scorep)[section].contains(option))
        (*scorep)[section].add_option(option, value);
    else
        (*scorep)[section][option].set<T>(value);
}

template<class T> T score_get(const string & section, const string & option, const T & default_value) {
    if (! scorep->contains(section))
        scorep->add_section(section);
    if (! (*scorep)[section].contains(option))
        (*scorep)[section].add_option(option, (T) default_value);
    return (*scorep)[section][option].get<T>();
}

template<class T> vector<T> score_get_list(const string &section, const string &option, const T &default_value) {
    if (!scorep->contains(section))
        scorep->add_section(section);
    if (! (*scorep)[section].contains(option))
        (*scorep)[section].add_option(option, default_value);
    return (*scorep)[section][option].get_list<T>();
}

void save_scores() {
    ofstream outfs;
    outfs.open(SCORE_FILE);
    outfs << *scorep;
    outfs.close();
}

void clear_top(Top *top) {
    for (int index = 0; index < TOP_MAX; index++) {
        top[index].nick = "---";
        top[index].score = 0;
    }
}

void read_top(Top *top, vector<string> &value) {
    clear_top(top);
    for (auto &row : value) {
        if (row.empty()) break;
        u_long separator = row.find(' ');
        top->nick = row.substr(0, separator);
        top->score = (u_long) strtol(row.substr(separator + 1).c_str(), nullptr, 10);
        top++;
    }
}

void read_tops() {

    // Create score parser
    if (!fexists(SCORE_FILE))
        save_scores();
    *scorep = parser::load_file(SCORE_FILE);

    vector<string> value;

    // Weekly top
    value = score_get_list<string>("Top", "Week", "");
    read_top(topWeek, value);

    // Yearly top
    value = score_get_list<string>("Top", "Year", "");
    read_top(topYear, value);
}

void write_top(Top *top, string &value) {
    for (int index = 0; index < TOP_MAX; index++) {
        if (top[index].score == 0) break;
        if (index > 0) value += ":";
        value += top[index].nick + " " + to_string(top[index].score);
    }
}

void write_tops() {
    string value;
    write_top(topWeek, value);

    // Score should already have the section / options created
    score_add<string>("Top", "Week", value);

    value = "";
    write_top(topYear, value);
    score_add<string>("Top", "Year", value);
}

bool update_top(Top *top, const string & nickname, u_long score) {
    int newPos, index;
    // Find where to insert new score
    for (newPos = 0; newPos < TOP_MAX; newPos++)
        if (score >= top[newPos].score)
            break;

    // Score did not make it into the top
    if (newPos == TOP_MAX) return false;

    // Check if player was already in the top, or top is not filled
    for (index = newPos; index < TOP_MAX; index++)
        if (top[index].nick == nickname || top[index].nick == "---")
            break;

    // Push tops down to make space
    for (; index > newPos; index--) {
        top[index].nick = top[index - 1].nick;
        top[index].score = top[index - 1].score;
    }

    // Finally, insert new top score
    top[newPos].nick = nickname;
    top[newPos].score = score;
    return true;
}

void get_scores(const string & nickname, u_long *year, u_long *week) {
    char *scan;
    string value = score_get<string>("Scores", "x" + nickname, "0 0");
    *year = strtoul(value.c_str(), &scan, 10);
    *week = strtoul(scan, &scan, 10);
}

void set_scores(const string & nickname, u_long year, u_long week) {
    string value = to_string(year) + " " + to_string(week);
    score_add<string>("Scores", "x" + nickname, value);
    bool updated = update_top(topWeek, nickname, week);
    updated |= update_top(topYear, nickname, year);
    if (updated) write_tops();
    save_scores();
}

void clear_week_scores() {
    for (auto & player : (*scorep)["Scores"]) {
        string score = player.get<string>();
        u_long len = score.find(' ');
        player.set<string>(score.substr(0, len + 1) + '0');
    }
    clear_top(topWeek);
    write_tops();
    save_scores();
}