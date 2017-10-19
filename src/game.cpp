//
// Created by invik on 17/10/17.
//

#include "game.h"
#include "dict_handler.h"
#include "scores_handler.h"

#include <ncurses.h>

enum {
    RUNNING, STOPPED, HALTING
} cur_state;

string lastWinner;
ulong winInARow;

void show_about() {
    char buffer[sizeof(ADVERTISE)];
    strcpy(buffer, ADVERTISE);
    irc_sendmsg(channel);
    if (irc_blackAndWhite) irc_stripcodes(buffer);
    irc_sendline(buffer);
}

void pickLetters(char *letters, char *sortedLetters) {
    size_t count = distrib.length();
    char availableLetters[count];
    strcpy(availableLetters, distrib.c_str());
    for (size_t index = 0; index < wordlen; index++) {
        ulong value;
        do {
            value = ((ulong) rand()) * count / RAND_MAX;
        } while (availableLetters[value] == 0);
        letters[index] = availableLetters[value];
        availableLetters[value] = 0;
    }
    letters[wordlen] = 0;
#ifdef CHEAT
    cout << "[CHEATING] Input letters" << endl;
    cin >> letters;
#endif
    sortLetters(letters, sortedLetters);
}

void displayLetters(const char *letters) {
    irc_sendmsg(channel);
    char s_letters[wordlen * 2];
    for (size_t i = 0; i < wordlen; i++) {
        s_letters[i * 2] = letters[i];
        s_letters[i * 2 + 1] = ' ';
    }
    s_letters[wordlen * 2 - 1] = '\0';
    irc_sendformat(true, "Letters", "[Mixed Letters]  - %s -", s_letters);
}

void send_update_stats(const string &nickname, ulong gain) {
    ulong year, week;
    get_scores(nickname, &year, &week);
    year += gain;
    week += gain;
    set_scores(nickname, year, week);

    if (strcasecmp(lastWinner.c_str(), nickname.c_str()) != 0) {
        winInARow = 1;
        irc_sendformat(true, "Stats", "( %d pts :  %d pts)", week, year);
    } else {
        winInARow++;
        irc_sendformat(true, "StatsCont", "( %d pts :  %d pts) - %d  contiguous won games!", winInARow, week, year);
    }
    lastWinner = nickname;
}

bool isWord(const char *letters, const char *word) {
    char ch = *letters++;
    const struct Cell *cell = dictionary;
    do {
        while (cell && (cell->letter < ch)) cell = cell->other;
        if (!cell) return false;
        if (cell->letter == ch) {
            ch = *letters++;
            if (ch == 0) {
                size_t len = strlen(word) + 1;
                for (ulong index = 0; index < cell->wordsCount; index++)
                    if (strcmp(cell->words + index * len, word) == 0)
                        return true;
                return false;
            } else
                cell = cell->longer;
        } else
            return false;
    } while (cell);
    return false;
}

bool checkWord(const char *availableLetters, const char *word) {
    char letters[wordlen];
    size_t len = strlen(word);
    if (len > wordlen) return false;
    sortLetters(word, letters);
    size_t scan = wordlen;
    while (len--) {
        do {
            if (scan == 0) return false;
        } while (availableLetters[--scan] > letters[len]);
        if (availableLetters[scan] != letters[len]) return false;
    }
    return isWord(letters, word);
}

bool is_owner(const string &nickname) {
    const char *scan = strcasestr(owner.c_str(), nickname.c_str());
    if (scan != nullptr) {
        if ((scan == owner) || (scan[-1] == ',')) {
            scan += nickname.length();
            if ((*scan == '\0') || (*scan == ','))
                return true;
        }
    }
    return false;
}

void replyScore(const char *nickname, const char *dest) {
    ulong year, week;
    get_scores(nickname, &year, &week);
    irc_sendnotice(dest);
    if (year == 0)
        irc_sendformat(true, "ScoreUnknown", "%s has never played with me.", nickname);
    else
        irc_sendformat(true, "Score", "%s's score is %d point(s) for this week, %d for this year.", nickname, week,
                       year);
}

void replyTop10(const char *dest, Top *top, const char *whichTop, const char *lpDefault, const char *whichTopMore,
                const char *lpDefaultMore) {
    irc_sendnotice(dest);
    irc_sendformat(true, whichTop, lpDefault, top[0].nick.c_str(), top[0].score);
    irc_sendnotice(dest);
    irc_sendformat(true, whichTopMore, lpDefaultMore, 2, top[1].nick.c_str(), top[1].score, 3, top[2].nick.c_str(),
                   top[2].score, 4, top[3].nick.c_str(), top[3].score);
    irc_sendnotice(dest);
    irc_sendformat(true, whichTopMore, lpDefaultMore, 5, top[4].nick.c_str(), top[4].score, 6, top[5].nick.c_str(),
                   top[5].score, 7, top[6].nick.c_str(), top[6].score);
    irc_sendnotice(dest);
    irc_sendformat(true, whichTopMore, lpDefaultMore, 8, top[7].nick.c_str(), top[7].score, 9, top[8].nick.c_str(),
                   top[8].score, 10, top[9].nick.c_str(), top[9].score);
}

void replyTop3(const char *dest, Top *top, const char *whichTop, const char *lpDefault) {
    irc_sendnotice(dest);
    irc_sendformat(true, whichTop, lpDefault, top[0].nick.c_str(), top[0].score, top[1].nick.c_str(), top[1].score,
                   top[2].nick.c_str(), top[2].score);
}

bool scrabbleCmd(const char *nickname, char *command) {
    bool isOwner = is_owner(nickname);
    if (strcasecmp(command, "!score") == 0)
        replyScore(nickname, nickname);
    else if (strncasecmp(command, "!score ", 7) == 0)
        replyScore(command + 7, nickname);
    else if ((strcasecmp(command, "!top") == 0) || (strcasecmp(command, "!top10") == 0))
        replyTop10(nickname, topWeek,
                   "TopW10", "The 10 best players of the week: 1. %s (%d)",
                   "TopW10More", "%d. %s (%d) - %d. %s (%d) - %d. %s (%d)");
    else if (strcasecmp(command, "!top10year") == 0)
        replyTop10(nickname, topYear,
                   "TopY10", "The 10 best players of the year: 1. %s (%d)",
                   "TopY10More", "%d. %s (%d) - %d. %s (%d) - %d. %s (%d)");
    else if (strcasecmp(command, "!top3") == 0)
        replyTop3(nickname, topWeek, "TopW3",
                  "The 3 best players of the week:  1. %s (%d) - 2. %s (%d) - 3. %s (%d)");
    else if (strcasecmp(command, "!top3year") == 0)
        replyTop3(nickname, topYear, "TopY3",
                  "The 3 best players of the year:  1. %s (%d) - 2. %s (%d) - 3. %s (%d)");
    else if (strcasecmp(command, "!start") == 0)
        cur_state = RUNNING;
    else if ((anyoneCanStop || isOwner) && (strcasecmp(command, "!stop") == 0)) {
        if (cur_state == RUNNING) {
            irc_sendmsg(channel);
            irc_sendformat(true, "Stop", "%s has stopped the game.", nickname);
        }
        cur_state = STOPPED;
    } else if (isOwner) {
        if (strcasecmp(command, "!newweek") == 0) {
            irc_sendmsg(channel);
            irc_sendformat(true, "NewWeek", "A new week is beginning ! Resetting all week scores...");
            clear_week_scores();
        } else if (strcasecmp(command, "!disconnect") == 0) {
            cur_state = HALTING;
        } else if (strcasecmp(command, "!op") == 0) {
            irc_sendline("MODE " + channel + " +o " + nickname);
        } else
            return false;
    } else
        return false;
    return true; // commande reconnue
}

void run_game() {

    cur_state = RUNNING;
    uint noWinner = 0;

    // Use global cfg reader to read cfg
    uint cfg_clock = (uint) cfg<unsigned_ini_t>("Delay", "max", 40) * 10;
    uint cfg_warning = (uint) cfg<unsigned_ini_t>("Delay", "warning", 30) * 10;
    uint cfg_after = (uint) cfg<unsigned_ini_t>("Delay", "after", 30) * 10;
    uint autostop = (uint) cfg<unsigned_ini_t>("Settings", "autostop", 3);
    bool autovoice = (bool) cfg<unsigned_ini_t>("Settings", "autovoice", 1);

    while (cur_state != HALTING) {
        char letters[wordlen], sortedLetters[wordlen];
        do {
            pickLetters(letters, sortedLetters);
            displayLetters(letters);
            findWords(sortedLetters);
            irc_sendmsg(channel);
            if (foundWords == 0)
                irc_sendformat(true, "FoundNone", "[I've found no possible words !! Let's roll again ;-)...]");
            else
                irc_sendformat(true, "Found", "[I've found %d words, including %d which contain %d letters.]",
                               foundWords, foundMaxWords, maxWordLen);
        } while (foundWords == 0);
        int tclock = cfg_clock;
        int warning = tclock - cfg_warning;
#ifdef CHEAT
        displayMaxWords(sortedLetters, maxWordLen);
        cout << dispMaxWordsString + 3 << endl;
        tclock = 1;
#endif
        time_t t = time(nullptr);
        struct tm *systemTime = localtime(&t);
        int lastDayOfWeek = systemTime->tm_wday;
        int lastHour = systemTime->tm_hour;
        char line[1024];
        char winningNick[128];                /////////////////////// revisar esto �String?
        size_t winningWordLen = 0;
        clock_t lastRecvTicks = clock();
        bool PINGed = false;
        do {
            tclock--;
            if (tclock == warning) { // plus que 10 sec
                irc_sendmsg(channel);
                irc_sendformat(true, "Warning", "Warning, time is nearly out!");
            } else if (tclock == 0) {
                irc_sendmsg(channel);
                displayMaxWords(sortedLetters, maxWordLen);
                irc_sendformat(true, "Timeout", "[Time is out.] MAX words were %s", dispMaxWordsString + 3);
                if (winningWordLen) {
                    irc_sendmsg(channel);
                    irc_sendformat(false, "WinSome", "%s gets %d points! ", winningNick, winningWordLen);
                    send_update_stats(winningNick, winningWordLen);
                    noWinner = 0;
                } else if (++noWinner == autostop) {
                    noWinner = 0;
                    irc_sendmsg(channel);
                    irc_sendformat(true, "GameOver", "[Game is over. Type !start to restart it.]");
                    cur_state = STOPPED;
                }
                break;
            }
            msleep(100);

            t = time(nullptr);
            systemTime = localtime(&t);
            if (systemTime->tm_wday != lastDayOfWeek) {
                if (systemTime->tm_wday == 1) { // we are now monday
                    irc_sendmsg(channel);
                    irc_sendformat(true, "NewWeek", "A new week is beginning ! Resetting all week scores...");
                    clear_week_scores();
                }
                lastDayOfWeek = systemTime->tm_wday;
            }
            if (systemTime->tm_hour != lastHour) {
                show_about();
                lastHour = systemTime->tm_hour;
            }

            if (kbhit()) {
                int k;
                if ((k = getch()) == 27) cur_state = HALTING;
                else cout << "key: " << k << endl;
            }
            while (irc_recv(line)) {
                lastRecvTicks = clock();
                PINGed = false;
                char *nickname, *ident, *hostname, *cmd, *param1, *param2, *paramtext;
                irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1, &param2, &paramtext);
                if ((strcmp(cmd, "PRIVMSG") == 0) && (strcasecmp(param1, channel.c_str()) == 0)) {
                    irc_stripcodes(paramtext);
                    while (isspace(*paramtext)) paramtext++;
                    if (*paramtext == 0) continue;
                    if (strncasecmp(paramtext, "!r", 2) == 0)
                        displayLetters(letters);
                    else if (!scrabbleCmd(nickname, paramtext)) {
                        while ((*paramtext != 0) && !is_valid_char(*paramtext)) paramtext++;
                        if (*paramtext == 0) continue;
                        char *wordEnd = paramtext + 1;
                        while (is_valid_char(*wordEnd)) wordEnd++;
                        *wordEnd = 0;
                        if (strlen(paramtext) > winningWordLen) {
                            strupr(paramtext);
                            if (checkWord(sortedLetters, paramtext)) {
                                irc_sendmsg(channel);
                                strcpy(winningNick, nickname);
                                winningWordLen = (int) strlen(paramtext);
                                if (winningWordLen == maxWordLen) {
                                    irc_sendformat(false, "Win",
                                                   "Congratulations %s ! There's not better [%s] !! You get %d points + %d bonus !",
                                                   paramtext, nickname, winningWordLen, bonus);
                                    send_update_stats(nickname, winningWordLen + bonus);
                                    if (autovoice) {
                                        irc_sendline("MODE " + channel + " +v " + nickname);
                                    }
                                    tclock = 0;
                                } else {
                                    irc_sendformat(true, "Word",
                                                   "Not bad %s... I keep your word [%s] ! Who can say better than %d letters ?",
                                                   paramtext, nickname, (int) strlen(paramtext));
                                }
                            }
                        }
                    }
                }
            }
            if (!PINGed && (clock() - lastRecvTicks > 15000)) {
                sprintf(line, "%.8X", (rand() << 16) | rand());
                irc_sendline("PING :" + (string) line);
                PINGed = true;
            } else if (PINGed && (clock() - lastRecvTicks > 20000)) {
                irc_disconnect();
                game_loop();
            }
        } while ((cur_state == RUNNING) && tclock);
        tclock = cfg_after;
        do {
            msleep(100);
            if (kbhit() && (getch() == 27)) cur_state = HALTING;
            while (irc_recv(line)) {
                char *nickname, *ident, *hostname, *cmd, *param1, *param2, *paramtext;
                irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1, &param2, &paramtext);
                if ((strcmp(cmd, "PRIVMSG") == 0) && (strcasecmp(param1, channel.c_str()) == 0)) {
                    irc_stripcodes(paramtext);
                    while (isspace(*paramtext)) paramtext++;
                    if (*paramtext == 0) continue;
                    scrabbleCmd(nickname, paramtext);
                }
            }
        } while (((cur_state == RUNNING) && tclock--) || (tclock = 0, cur_state == STOPPED));
    }
}

void game_loop() {
    // Connect to irc
    cout << "Connecting to IRC..." << endl;
    irc_connect();

    // Init execution
    show_about();
    run_game();

    irc_disconnect();
}