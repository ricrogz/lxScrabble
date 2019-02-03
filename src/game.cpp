//
// Created by invik on 17/10/17.
//

#include <random>

#include "bot_commands.hpp"
#include "dict_handler.hpp"
#include "game.hpp"
#include "mimics.hpp"
#include "scores_handler.hpp"

#include <algorithm>

using RandGenerator = std::default_random_engine;

run_state cur_state;
std::string lastWinner;
std::size_t winInARow;
time_t last_msg; // Updates on last channel msg, when not playing

void show_about()
{
    std::string buffer(ADVERTISE);
    irc_sendmsg(channel);
    if (irc_blackAndWhite) {
        buffer = irc_stripcodes(ADVERTISE);
    }
    irc_sendline(buffer);
    irc_sendmsg(channel);
    irc_sendformat(true, "Help",
                   "Use '!help' to ask the bot for available commands");
}

std::string pickLetters(RandGenerator& get_rand_int,
                        const std::string& availableLetters)
{
#ifdef CHEAT
    std::string letters;
    log_stdout("[CHEATING] Input letters");
    std::cin >> letters;
#else
    std::string letters(availableLetters);
    std::shuffle(letters.begin(), letters.end(), get_rand_int);
    letters.resize(wordlen);
#endif
    return std::move(letters);
}

void displayLetters(const std::string& letters)
{
    irc_sendmsg(channel);
    std::ostringstream ss;
    ss << ' ';
    for (const auto& l : letters) {
        ss << l << ' ';
    }
    irc_sendformat(true, "Letters", "[Mixed Letters]  - %s -",
                   ss.str().c_str());
}

void send_update_stats(const std::string& nickname, unsigned long gain)
{
    unsigned long year, week;
    get_scores(nickname, &year, &week);
    year += gain;
    week += gain;
    set_scores(nickname, year, week);

    if (strcasecmp(lastWinner.c_str(), nickname.c_str()) != 0) {
        winInARow = 1;
        irc_sendformat(true, "Stats", "( %d pts :  %d pts)", week, year);
    } else {
        winInARow++;
        irc_sendformat(true, "StatsCont",
                       "( %d pts :  %d pts) - %d  contiguous won games!",
                       winInARow, week, year);
    }
    lastWinner = nickname;
}

bool isWord(Cell const* dictionary, const std::string& letters,
            const std::string& word)
{
    auto char_ptr = letters.begin();
    Cell const* cell = dictionary;
    do {
        while (cell && cell->letter < *char_ptr) {
            cell = cell->other;
        }
        if (!cell) {
            return false;
        }
        if (cell->letter == *char_ptr) {
            if (++char_ptr == letters.end()) {
                return cell->words.end() !=
                       std::find(cell->words.begin(), cell->words.end(), word);
            } else {
                cell = cell->longer;
            }
        } else {
            return false;
        }
    } while (cell);
    return false;
}

bool isPossible(std::size_t wordlen, const std::string& availableLetters,
                const std::string& sortedWord)
{
    if (sortedWord.size() > wordlen) {
        return false;
    }

    auto available = availableLetters.begin();
    for (auto l = sortedWord.begin(); l != sortedWord.end();) {
        while (*l > *available) {
            if (++available == availableLetters.end()) {
                return false;
            }
        }
        if (*l < *available) {
            return false;
        }
        if (++l != sortedWord.end() && ++available == availableLetters.end()) {
            return false;
        }
    }
    return true;
}

bool is_owner(const std::string& nickname)
{
    for (auto& o : owner)
        if (strcasecmp(o.c_str(), nickname.c_str()) == 0)
            return true;
    return false;
}

void replyScore(const char* nickname, const char* dest)
{
    unsigned long year, week;
    get_scores(nickname, &year, &week);
    irc_sendnotice(dest);
    if (year == 0)
        irc_sendformat(true, "ScoreUnknown", "%s has never played with me.",
                       nickname);
    else
        irc_sendformat(
            true, "Score",
            "%s's score is %d point(s) for this week, %d for this year.",
            nickname, week, year);
}

void replyTop10(const char* dest, Top* top, const char* whichTop,
                const char* lpDefault, const char* whichTopMore,
                const char* lpDefaultMore)
{
    irc_sendnotice(dest);
    irc_sendformat(true, whichTop, lpDefault, &(top[0].nick)[0], top[0].score);
    irc_sendnotice(dest);
    irc_sendformat(true, whichTopMore, lpDefaultMore, 2, &(top[1].nick)[0],
                   top[1].score, 3, &(top[2].nick)[0], top[2].score, 4,
                   &(top[3].nick)[0], top[3].score);
    irc_sendnotice(dest);
    irc_sendformat(true, whichTopMore, lpDefaultMore, 5, &(top[4].nick)[0],
                   top[4].score, 6, &(top[5].nick)[0], top[5].score, 7,
                   &(top[6].nick)[0], top[6].score);
    irc_sendnotice(dest);
    irc_sendformat(true, whichTopMore, lpDefaultMore, 8, &(top[7].nick)[0],
                   top[7].score, 9, &(top[8].nick)[0], top[8].score, 10,
                   &(top[9].nick)[0], top[9].score);
}

void replyTop3(const char* dest, Top* top, const char* whichTop,
               const char* lpDefault)
{
    irc_sendnotice(dest);
    irc_sendformat(true, whichTop, lpDefault, &(top[0].nick)[0], top[0].score,
                   &(top[1].nick)[0], top[1].score, &(top[2].nick)[0],
                   top[2].score);
}

bool scrabbleCmd(const char* nickname, char* command)
{
    bool isOwner = is_owner(nickname);
    if (strncasecmp(command, "!help", 5) == 0)
        help_cmd(nickname, isOwner);
    else if (strcasecmp(command, "!score") == 0)
        replyScore(nickname, nickname);
    else if (strncasecmp(command, "!score ", 7) == 0)
        replyScore(command + 7, nickname);
    else if ((strcasecmp(command, "!top") == 0) ||
             (strcasecmp(command, "!top10") == 0))
        replyTop10(nickname, topWeek, "TopW10",
                   "The 10 best players of the week: 1. %s (%d)", "TopW10More",
                   "%d. %s (%d) - %d. %s (%d) - %d. %s (%d)");
    else if (strcasecmp(command, "!top10year") == 0)
        replyTop10(nickname, topYear, "TopY10",
                   "The 10 best players of the year: 1. %s (%d)", "TopY10More",
                   "%d. %s (%d) - %d. %s (%d) - %d. %s (%d)");
    else if (strcasecmp(command, "!top3") == 0)
        replyTop3(nickname, topWeek, "TopW3",
                  "The 3 best players of the week:  1. %s (%d) - 2. %s (%d) - "
                  "3. %s (%d)");
    else if (strcasecmp(command, "!top3year") == 0)
        replyTop3(nickname, topYear, "TopY3",
                  "The 3 best players of the year:  1. %s (%d) - 2. %s (%d) - "
                  "3. %s (%d)");
    else if (strcasecmp(command, "!start") == 0)
        cur_state = RUNNING;
    else if ((anyoneCanStop || isOwner) &&
             (strcasecmp(command, "!stop") == 0)) {
        if (cur_state == RUNNING) {
            irc_sendmsg(channel);
            irc_sendformat(true, "Stop", "%s has stopped the game.", nickname);
            time(&last_msg);
        }
        cur_state = STOPPED;
    } else if (isOwner) {
        if (strcasecmp(command, "!newweek") == 0) {
            irc_sendmsg(channel);
            irc_sendformat(
                true, "NewWeek",
                "A new week is beginning ! Resetting all week scores...");
            clear_week_scores();
        } else if (strncasecmp(command, &("!quit " + public_nick)[0],
                               6 + public_nick.length()) == 0) {
            cur_state = QUITTING;
        } else if (strcasecmp(command, "!op") == 0) {
            irc_sendline("MODE " + channel + " +o " + nickname);
        } else
            return false;
    } else
        return false;
    return true; // commande reconnue
}

void run_game(Cell const* dictionary)
{

    cur_state = STOPPED;
    std::size_t noWinner = 0;

    RandGenerator get_rand_int(time(nullptr));

    while (cur_state != QUITTING) {

        char line[1024];

        int tclock = cfg_after;
        bool PINGed = false;
        clock_t lastRecvTicks = clock();

        time_t t;
        time(&t);
        struct tm* systemTime = localtime(&t);
        int lastDayOfWeek = systemTime->tm_wday;

        do {
            msleep(100);
            while (irc_recv(line)) {
                lastRecvTicks = clock();
                PINGed = false;
                char *nickname, *ident, *hostname, *cmd, *param1, *param2,
                    *paramtext;
                irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1,
                            &param2, &paramtext);
                if ((strcmp(cmd, "PRIVMSG") == 0) &&
                    (strcasecmp(param1, channel.c_str()) == 0)) {
                    time(&last_msg);
                    std::string ptext(paramtext);
                    ptext = irc_stripcodes(ptext);
                    if (ptext.empty()) {
                        continue;
                    }
                    scrabbleCmd(nickname, paramtext);

                    // Reanounce on JOIN after x time without noone talking
                } else if (reannounce > 0 && (strcmp(cmd, "JOIN") == 0) &&
                           (strcasecmp(paramtext, channel.c_str()) == 0)) {
                    time_t now;
                    time(&now);
                    if (now - last_msg > reannounce)
                        show_about();
                }
            }

            // Keep alive
            if (!PINGed && (clock() - lastRecvTicks > 15000)) {
                sprintf(line, "%.8X",
                        static_cast<unsigned int>((get_rand_int() << 16) |
                                                  get_rand_int()));
                irc_sendline("PING :" + std::string(line));
                PINGed = true;
            } else if (PINGed && (clock() - lastRecvTicks > TIMEOUT)) {
                log_stdout("***** Timeout detected, reconnecting. *****");
                irc_disconnect_msg("Timeout detected, reconnecting.");
                game_loop(dictionary);
            }

            // Reset weekly scores on monday 00:00
            time(&t);
            systemTime = localtime(&t);

            // Weekly score reset
            if (systemTime->tm_wday != lastDayOfWeek) {
                if (systemTime->tm_wday == 1) { // we are now monday
                    irc_sendmsg(channel);
                    irc_sendformat(true, "NewWeek",
                                   "A new week is beginning ! Resetting all "
                                   "week scores...");
                    clear_week_scores();
                }
                lastDayOfWeek = systemTime->tm_wday;
            }

        } while (((cur_state == RUNNING) && tclock--) ||
                 (tclock = 0, cur_state == STOPPED));

        // Check if quitting
        if (cur_state == QUITTING)
            return;

        /*
         * Start a new round: pick letters, find words, print letters & stats
         */

        std::string letters;
        std::string sortedLetters;
        FoundWords foundWords;
        do {
            letters = pickLetters(get_rand_int, distrib);
            displayLetters(letters);

            sortedLetters = std::string(letters);
            std::sort(sortedLetters.begin(), sortedLetters.end());

            foundWords = findWords(dictionary, sortedLetters);
            irc_sendmsg(channel);
            if (foundWords.totalWords == 0) {
                irc_sendformat(true, "FoundNone",
                               "[I've found no possible words !! Let's roll "
                               "again ;-)...]");
            } else {
                irc_sendformat(true, "Found",
                               "[I've found %d words, including %d which "
                               "contain %d letters.]",
                               foundWords.totalWords,
                               foundWords.bestWords.size(),
                               foundWords.lenBestWords);
            }
        } while (foundWords.totalWords == 0);
        tclock = cfg_clock;
        int warning = tclock - cfg_warning;
#ifdef CHEAT
        std::ostringstream ss;
        ss << "Longest Words :" << foundWords.maxWordsString << std::endl;
        log_stdout(ss.str());
#endif
        time(&t);
        systemTime = localtime(&t);
        lastDayOfWeek = systemTime->tm_wday;
        int lastHour = systemTime->tm_hour;
        char winningNick[128];
        std::size_t winningWordLen = 0;
        lastRecvTicks = clock();
        PINGed = false;
        do {
            tclock--;
            if (tclock == warning) { // plus que 10 sec
                irc_sendmsg(channel);
                irc_sendformat(true, "Warning", "Warning, time is nearly out!");
            } else if (tclock == 0) {
                irc_sendmsg(channel);
                std::ostringstream ss;
                ss << foundWords.bestWords.front();
                for (auto w = foundWords.bestWords.begin() + 1;
                     w != foundWords.bestWords.end(); ++w) {
                    ss << " - " << *w;
                }
                irc_sendformat(true, "Timeout",
                               "[Time is out.] MAX words were %s",
                               ss.str().c_str());
                if (winningWordLen) {
                    irc_sendmsg(channel);
                    irc_sendformat(false, "WinSome", "%s gets %d points! ",
                                   winningNick, winningWordLen);
                    send_update_stats(winningNick, winningWordLen);
                    noWinner = 0;
                } else if (++noWinner == autostop) {
                    noWinner = 0;
                    irc_sendmsg(channel);
                    irc_sendformat(
                        true, "GameOver",
                        "[Game is over. Type !start to restart it.]");
                    cur_state = STOPPED;
                    time(&last_msg);
                }
                break;
            }
            msleep(100);

            time(&t);
            systemTime = localtime(&t);

            // Weekly score reset
            if (systemTime->tm_wday != lastDayOfWeek) {
                if (systemTime->tm_wday == 1) { // we are now monday
                    irc_sendmsg(channel);
                    irc_sendformat(true, "NewWeek",
                                   "A new week is beginning ! Resetting all "
                                   "week scores...");
                    clear_week_scores();
                }
                lastDayOfWeek = systemTime->tm_wday;
            }

            // Announce bot banner
            if (systemTime->tm_hour != lastHour) {
                show_about();
                lastHour = systemTime->tm_hour;
            }

            while (irc_recv(line)) {
                lastRecvTicks = clock();
                PINGed = false;
                char *nickname, *ident, *hostname, *cmd, *param1, *param2,
                    *paramtext;
                irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1,
                            &param2, &paramtext);
                if ((strcmp(cmd, "PRIVMSG") == 0) &&
                    (strcasecmp(param1, channel.c_str()) == 0)) {
                    irc_stripcodes(paramtext);
                    while (isspace(*paramtext))
                        paramtext++;
                    if (*paramtext == 0)
                        continue;
                    if (strncasecmp(paramtext, "!r", 2) == 0)
                        displayLetters(letters);
                    else if (!scrabbleCmd(nickname, paramtext)) {
                        while ((*paramtext != 0) && !is_valid_char(*paramtext))
                            paramtext++;
                        if (*paramtext == 0)
                            continue;
                        if (strlen(paramtext) > winningWordLen) {
                            non_ascii_strupr(paramtext);
                            std::string sortedWord(paramtext);
                            std::sort(sortedWord.begin(), sortedWord.end());
                            if (isPossible(wordlen, sortedLetters,
                                           sortedWord) &&
                                isWord(dictionary, sortedWord, paramtext)) {
                                irc_sendmsg(channel);
                                strcpy(winningNick, nickname);
                                winningWordLen = strlen(paramtext);
                                if (winningWordLen == foundWords.lenBestWords) {
                                    irc_sendformat(
                                        false, "Win",
                                        "Congratulations %s ! There's not "
                                        "better [%s] !! You get %d points + %d "
                                        "bonus !",
                                        paramtext, nickname, winningWordLen,
                                        bonus);
                                    send_update_stats(nickname,
                                                      winningWordLen + bonus);
                                    if (autovoice) {
                                        irc_sendline("MODE " + channel +
                                                     " +v " + nickname);
                                    }
                                    tclock = 0;
                                } else {
                                    irc_sendformat(
                                        true, "Word",
                                        "Not bad %s... I keep your word [%s] ! "
                                        "Who can say better than %d letters ?",
                                        paramtext, nickname, strlen(paramtext));
                                }
                            }
                        }
                    }
                }
            }

            // Keep alive
            if (!PINGed && (clock() - lastRecvTicks > 15000)) {
                sprintf(line, "%.8X",
                        static_cast<unsigned int>((get_rand_int() << 16) |
                                                  get_rand_int()));
                irc_sendline("PING :" + std::string(line));
                PINGed = true;
            } else if (PINGed && (clock() - lastRecvTicks > TIMEOUT)) {
                log_stdout("***** Timeout detected, reconnecting. *****");
                irc_disconnect_msg("Timeout detected, reconnecting.");
                game_loop(dictionary);
            }

        } while ((cur_state == RUNNING) && tclock);
    }
}

void game_loop(Cell const* dictionary)
{
    // Connect to irc
    log_stdout("Connecting to IRC...");
    irc_connect();

    // Init execution
    show_about();
    run_game(dictionary);

    log_stdout("Quitting...");
    irc_disconnect();
}
