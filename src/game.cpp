//
// Created by invik on 17/10/17.
//

#include "game.h"
#include "dict_handler.h"
#include "scores_handler.h"

#include <cstring>
#include <unistd.h>

void show_about() {
    char buffer[sizeof(ADVERTISE)];
    strcpy(buffer, ADVERTISE);
    irc_sendmsg(channel.c_str());
    if (irc_blackAndWhite) irc_stripcodes(buffer);
    irc_sendline(buffer);
}

void pickLetters(char letters[WORD_MAX], char sortedLetters[WORD_MAX]) {
    size_t count = distrib.length();
    char availableLetters[count];
    strcpy(availableLetters, distrib.c_str());
    for (size_t index = 0; index < wordlen; index++) {
        unsigned long value;
        do {
            value = ((unsigned long) rand()) * count / RAND_MAX;
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

void displayLetters(const char letters[WORD_MAX]) {
    irc_sendmsg(channel.c_str());
    char s_letters[WORD_MAX * 2];
    for (size_t i = 0; i < wordlen; i++) {
        s_letters[i * 2] = letters[i];
        s_letters[i * 2 + 1] = ' ';
    }
    s_letters[wordlen * 2 - 1] = '\0';
    irc_sendformat(true, "Letters", "[Mixed Letters]  - %s -", s_letters);
}

/*
void send_update_stats(const char *nickname, int gain) {
    int year, week;
    get_scores(nickname, &year, &week);
    year += gain;
    week += gain;
    set_scores(nickname, year, week);

    if (stricmp(lastWinner, nickname) != 0) {
        winInARow = 1;
        irc_sendformatline("Stats", "( %1!d! pts :  %2!d! pts)", week, year);
    } else {
        winInARow++;
        irc_sendformatline("StatsCont", "( %1!d! pts :  %2!d! pts) - %3!d!  contiguous won games!", week, year,
                           winInARow);
    }
    strcpy(lastWinner, nickname);
}
*/

void run_game() {

    enum {
        RUNNING, STOPPED, HALTING
    } cur_state = RUNNING;
    const char *chan = channel.c_str();
    uint noWinner = 0;

    // Use global cfg reader to read cfg
    int cfg_clock = (int)cfg<unsigned_ini_t>("Delay", "max", 40) * 10;
    int cfg_warning = (int)cfg<unsigned_ini_t>("Delay", "warning", 30) * 10;
    int cfg_after = (int)cfg<unsigned_ini_t>("Delay", "after", 30) * 10;
    int autostop = (int)cfg<unsigned_ini_t>("Settings", "autostop", 3);
    bool autovoice = (bool)cfg<unsigned_ini_t>("Settings", "autovoice", true);

    while (cur_state != HALTING) {
        char letters[WORD_MAX], sortedLetters[WORD_MAX];
        do {
            pickLetters(letters, sortedLetters);
            displayLetters(letters);
            findWords(sortedLetters);
            irc_sendmsg(chan);
            if (foundWords == 0)
                irc_sendformat(true, "FoundNone", "[I've found no possible words !! Let's roll again ;-)...]");
            else
                irc_sendformat(true, "Found", "[I've found %d words, including %d which contain %d letters.]",
                               foundWords, foundMaxWords, maxWordLen);
        } while (foundWords == 0);
        int tclock = cfg_clock;
#ifdef CHEAT
        displayMaxWords(sortedLetters, maxWordLen);
        cout << dispMaxWordsString + 3 << endl;
        tclock = 1;
#endif
        int warning = tclock - cfg_warning;
        time_t t = time(0);
        struct tm *systemTime = localtime(&t);
        int lastDayOfWeek = systemTime->tm_wday;
        int lastHour = systemTime->tm_hour;
        char line[1024];
        char winningNick[128];                /////////////////////// revisar esto ¿String?
        int winningWordLen = 0;
        clock_t lastRecvTicks = clock();
        bool PINGed = false;
        do {
            tclock--;
            if (tclock == warning) { // plus que 10 sec
                irc_sendmsg(chan);
                irc_sendformat(true, "Warning", "Warning, time is nearly out!");
            } else if (tclock == 0) {
                irc_sendmsg(chan);
                displayMaxWords(sortedLetters, maxWordLen);
                irc_sendformat(true, "Timeout", "[Time is out.] MAX words were %s", dispMaxWordsString + 3);
                if (winningWordLen) {
                    irc_sendmsg(chan);
                    irc_sendformat(false, "WinSome", "%s gets %d points! ", winningNick, winningWordLen);
                    send_update_stats(winningNick, winningWordLen);
                    noWinner = 0;
                } else if (++noWinner == autostop) {
                    noWinner = 0;
                    irc_sendmsg(chan);
                    irc_sendformat(true, "GameOver", "[Game is over. Type !start to restart it.]");
                    cur_state = STOPPED;
                }
                break;
            }
            usleep(100);
            GetSystemTime(&systemTime);
            if (systemTime.wDayOfWeek != lastDayOfWeek) {
                if (systemTime.wDayOfWeek == 1) { // we are now monday
                    irc_sendmsg(chan);
                    irc_sendformat(true, "NewWeek", "A new week is beginning ! Resetting all week scores...");
                    clear_week_scores();
                }
                lastDayOfWeek = systemTime.wDayOfWeek;
            }
            if (systemTime.wHour != lastHour) {
                show_about();
                lastHour = systemTime.wHour;
            }

            if (kbhit() && (getch() == 27)) cur_state = HALTING;
            while (irc_recv(line)) {
                lastRecvTicks = clock();
                PINGed = false;
                char *nickname, *ident, *hostname, *cmd, *param1, *param2, *paramtext;
                irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1, &param2, &paramtext);
                if ((strcmp(cmd, "PRIVMSG") == 0) && (stricmp(param1, chan) == 0)) {
                    irc_stripcodes(paramtext);
                    while (isspace(*paramtext)) paramtext++;
                    if (*paramtext == 0) continue;
                    if (strnicmp(paramtext, "!r", 2) == 0)
                        displayLetters(letters);
                    else if (!scrabbleCmd(nickname, paramtext)) {
                        while ((*paramtext != 0) && !isalpha(*paramtext)) paramtext++;
                        if (*paramtext == 0) continue;
                        char *wordEnd = paramtext + 1;
                        while (isalpha(*wordEnd)) wordEnd++;
                        *wordEnd = 0;
                        if ((int) strlen(paramtext) > winningWordLen) {
                            strupr(paramtext);
                            if (checkWord(sortedLetters, paramtext)) {
                                irc_sendmsg(chan);
                                strcpy(winningNick, nickname);
                                winningWordLen = (int) strlen(paramtext);
                                if (winningWordLen == maxWordLen) {
                                    irc_sendformat(false, "Win",
                                                   "Congratulations %s ! There's not better [%s] !! You get %d points + %d bonus !",
                                                   nickname, paramtext, winningWordLen, bonus);
                                    send_update_stats(nickname, winningWordLen + bonus);
                                    if (autovoice) {
                                        irc_send("MODE ");
                                        irc_send(chan);
                                        irc_send(" +v ");
                                        irc_sendline(nickname);
                                    }
                                    tclock = 0;
                                } else {
                                    irc_sendformat(true, "Word",
                                                   "Not bad %s... I keep your word [%s] ! Who can say better than %d letters ?",
                                                   nickname, paramtext, (int) strlen(paramtext));
                                }
                            }
                        }
                    }
                }
            }
            if (!PINGed && (clock() - lastRecvTicks > 15000)) {
                sprintf(line, "%.8X", (rand() << 16) | rand());
                irc_send("PING :");
                irc_sendline(line);
                PINGed = true;
            } else if (PINGed && (clock() - lastRecvTicks > 20000)) {
                irc_disconnect();
                game_loop();
            }
        } while ((cur_state == RUNNING) && tclock);
        tclock = cfg_after;
        do {
            usleep(100);
            if (kbhit() && (getch() == 27)) cur_state = HALTING;
            while (irc_recv(line)) {
                char *nickname, *ident, *hostname, *cmd, *param1, *param2, *paramtext;
                irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1, &param2, &paramtext);
                if ((strcmp(cmd, "PRIVMSG") == 0) && (stricmp(param1, chan) == 0)) {
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