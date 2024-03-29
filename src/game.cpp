//
// Created by invik on 17/10/17.
//

#include <algorithm>
#include <cstring>
#include <iostream>

#include "fmt/ranges.h"

#include "bot_commands.hpp"
#include "dict_handler.hpp"
#include "game.hpp"
#include "irc.hpp"
#include "mimics.hpp"
#include "scoreboard.hpp"

#ifdef CHEAT
const bool cheats_enabled = true;
#else
const bool cheats_enabled = false;
#endif

extern Scoreboard* scores;

run_state cur_state = STOPPED;
std::string lastWinner;
size_t winInARow;
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
    std::string letters(availableLetters);
    if (cheats_enabled) {
        log("[cheats_enabled] Input letters");
        std::cin >> letters;
    } else {
        std::shuffle(letters.begin(), letters.end(), get_rand_int);
        letters.resize(wordlen);
    }
    return letters;
}

void displayLetters(const std::string& letters)
{
    irc_sendmsg(channel);
    auto msg = fmt::format(" {} ", fmt::join(letters, " "));
    irc_sendformat(true, "Letters", "[Mixed Letters]  - {:s} -", msg);
}

void send_update_stats(const std::string& nickname, unsigned long gain)
{
    scores->add_score(nickname, gain);
    const auto week = scores->get_score(nickname, Scoreboard::Type::Week);
    const auto alltime = scores->get_score(nickname, Scoreboard::Type::Total);

    if (strcasecmp(lastWinner.c_str(), nickname.c_str()) != 0) {
        winInARow = 1;
        irc_sendformat(true, "Stats", "( {:d} pts :  {:d} pts)", week, alltime);
    } else {
        ++winInARow;
        irc_sendformat(true, "StatsCont",
                       "( {:d} pts :  {:d} pts) - {:d}  contiguous won games!",
                       winInARow, week, alltime);
    }
    lastWinner = nickname;
}

bool isWord(Cell const* dictionary, const std::string& letters,
            const std::string& word)
{
    auto char_ptr = letters.begin();
    Cell const* cell = dictionary;
    do {
        while ((cell != nullptr) && cell->letter < *char_ptr) {
            cell = cell->other;
        }
        if (cell == nullptr) {
            return false;
        }
        if (cell->letter == *char_ptr) {
            if (++char_ptr == letters.end()) {
                return cell->words.end() !=
                       std::find(cell->words.begin(), cell->words.end(), word);
            }
            cell = cell->longer;
        } else {
            return false;
        }
    } while (cell != nullptr);
    return false;
}

bool isPossible(size_t wordlen, const std::string& availableLetters,
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
    for (auto& o : owner) {
        if (strcasecmp(o.c_str(), nickname.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

void replyScore(const char* nickname, const char* dest)
{
    const auto week = scores->get_score(nickname, Scoreboard::Type::Week);
    const auto alltime = scores->get_score(nickname, Scoreboard::Type::Total);

    irc_sendnotice(dest);
    if (alltime == 0UL) {
        irc_sendformat(true, "ScoreUnknown", "{:s} has never played with me.",
                       nickname);
    } else {
        irc_sendformat(
            true, "Score",
            "{:s}'s score is {:d} point(s) for this week, {:d} in total.",
            nickname, week, alltime);
    }
}

void sendTop(const char* dest, Scoreboard::Type which, size_t num,
             const char* lpDefault, const char* lpDefaultMore)
{
    if (num != 3) {
        num = 1 + num - (num % 3);
    }

    auto title = fmt::format("Top{}{}",
                             which == Scoreboard::Type::Week ? 'W' : 'A', num);
    const auto top = scores->get_top(which, num);

    auto top_iter = top.begin();

    irc_sendnotice(dest);
    if (num == 3) {
        auto a = top_iter;
        auto b = ++top_iter;
        auto c = ++top_iter;
        irc_sendformat(true, title, lpDefault, a->first, a->second, b->first,
                       b->second, c->first, c->second);
    } else {
        irc_sendformat(true, title, lpDefault, top_iter->first,
                       top_iter->second);

        title += "More";
        for (size_t i = 2; i < num; i += 3) {
            auto a = ++top_iter;
            auto b = ++top_iter;
            auto c = ++top_iter;
            irc_sendnotice(dest);
            irc_sendformat(true, title, lpDefaultMore, i, a->first, a->second,
                           i + 1, b->first, b->second, i + 2, c->first,
                           c->second);
        }
    }
}

bool scrabbleCmd(const char* nickname, const char* command)
{
    bool isOwner = is_owner(nickname);
    if (strncasecmp(command, "!help", 5) == 0) {
        help_cmd(nickname, isOwner);
    } else if (strcasecmp(command, "!score") == 0) {
        replyScore(nickname, nickname);
    } else if (strncasecmp(command, "!score ", 7) == 0) {
        replyScore(&command[7], nickname);
    } else if (strcasecmp(command, "!top") == 0 ||
               strcasecmp(command, "!top10") == 0) {
        sendTop(nickname, Scoreboard::Type::Week, 10,
                "The 10 best players of the week: 1. {:s} ({:d})",
                "{:d}. {:s} ({:d}) - {:d}. {:s} ({:d}) - {:d}. {:s} ({:d})");
    } else if (strcasecmp(command, "!top10total") == 0) {
        sendTop(nickname, Scoreboard::Type::Total, 10,
                "The 10 best players of the all time: 1. {:s} ({:d})",
                "{:d}. {:s} ({:d}) - {:d}. {:s} ({:d}) - {:d}. {:s} ({:d})");
    } else if (strcasecmp(command, "!top3") == 0) {
        sendTop(nickname, Scoreboard::Type::Week, 3,
                "The 3 best players of the week:  1. {:s} ({:d}) - 2. {:s} "
                "({:d}) - 3. {:s} ({:d})",
                nullptr);
    } else if (strcasecmp(command, "!top3total") == 0) {
        sendTop(nickname, Scoreboard::Type::Total, 3,
                "The 3 best players of the all time:  1. {:s} ({:d}) - 2. {:s} "
                "({:d}) - 3. {:s} ({:d})",
                nullptr);
    } else if (strcasecmp(command, "!start") == 0) {
        cur_state = RUNNING;
    } else if ((anyoneCanStop || isOwner) &&
               (strcasecmp(command, "!stop") == 0)) {
        if (cur_state == RUNNING) {
            irc_sendmsg(channel);
            irc_sendformat(true, "Stop", "{:s} has stopped the game.",
                           nickname);
            time(&last_msg);
        }
        cur_state = STOPPED;
    } else if (isOwner) {
        if (strcasecmp(command, "!newweek") == 0) {
            irc_sendmsg(channel);
            irc_sendformat(
                true, "NewWeek",
                "A new week is beginning ! Resetting all week scores...");
            scores->clear(Scoreboard::Type::Week);
        } else if (strncasecmp(command, &("!quit " + public_nick)[0],
                               6 + public_nick.length()) == 0) {
            cur_state = QUITTING;
        } else if (strcasecmp(command, "!op") == 0) {
            irc_sendline("MODE " + channel + " +o " + nickname);
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true; // commande reconnue
}

void run_game(Cell const* dictionary)
{

    cur_state = STOPPED;
    size_t noWinner = 0;

    RandGenerator rnd_gen(time(nullptr));

    while (cur_state != QUITTING) {

        auto tclock = cfg_after;

        time_t t = 0;
        time(&t);
        struct tm* systemTime = localtime(&t);
        int lastDayOfWeek = systemTime->tm_wday;

        Pinger pinger(rnd_gen);

        std::string line;
        do {
            msleep(100);
            while (irc_recv(line)) {
                pinger.recv();
                std::string nickname;
                std::string dummy;
                std::string cmd;
                std::string msg_channel;
                std::string paramtext;
                irc_analyze(std::move(line), nickname, cmd, msg_channel,
                            paramtext);
                if (cmd == "PRIVMSG" &&
                    (strcasecmp(msg_channel.c_str(), channel.c_str()) == 0)) {
                    time(&last_msg);
                    paramtext = irc_stripcodes(paramtext);
                    if (paramtext.empty()) {
                        continue;
                    }
                    scrabbleCmd(nickname.c_str(), paramtext.c_str());

                    // Reannounce on JOIN after x time without no one talking
                } else if (reannounce > 0 && cmd == "JOIN" &&
                           (strcasecmp(paramtext.c_str(), channel.c_str()) ==
                            0)) {
                    time_t now = 0;
                    time(&now);
                    if (now - last_msg > reannounce) {
                        show_about();
                    }
                }
            }

            // Keep alive
            if (!pinger.is_alive()) {
                log("***** Timeout detected, reconnecting. *****");
                irc_disconnect_msg("Timeout detected, reconnecting.");
                return;
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
                    scores->clear(Scoreboard::Type::Week);
                }
                lastDayOfWeek = systemTime->tm_wday;
            }

        } while ((cur_state == RUNNING && ((tclock--) != 0u)) ||
                 (tclock = 0, cur_state == STOPPED));

        // Check if quitting
        if (cur_state == QUITTING) {
            return;
        }

        /*
         * Start a new round: pick letters, find words, print letters & stats
         */

        std::string letters;
        std::string sortedLetters;
        FoundWords foundWords;
        do {
            letters = pickLetters(rnd_gen, distrib);
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
                               "[I've found {:d} words, including {:d} which "
                               "contain {:d} letters.]",
                               foundWords.totalWords,
                               foundWords.bestWords.size(),
                               foundWords.lenBestWords);
            }
        } while (foundWords.totalWords == 0);
        tclock = cfg_clock;
        auto warning = tclock - cfg_warning;

        if (cheats_enabled) {
            log(fmt::format("\nLongest Words :{}\n", foundWords.bestWords));
        }
        time(&t);
        systemTime = localtime(&t);
        lastDayOfWeek = systemTime->tm_wday;
        int lastHour = systemTime->tm_hour;
        std::string winningNick;
        size_t winningWordLen = 0;
        pinger.recv();
        do {
            --tclock;
            if (tclock == warning) { // more than 10 s
                irc_sendmsg(channel);
                irc_sendformat(true, "Warning", "Warning, time is nearly out!");
            } else if (tclock == 0) {
                irc_sendmsg(channel);
                auto words =
                    fmt::format("{}", fmt::join(foundWords.bestWords, " - "));
                irc_sendformat(true, "Timeout",
                               "[Time is out.] MAX words were {:d}", words);
                if (winningWordLen != 0u) {
                    irc_sendmsg(channel);
                    irc_sendformat(false, "WinSome", "{:s} gets {:d} points! ",
                                   winningNick, winningWordLen);
                    send_update_stats(winningNick, winningWordLen);
                    noWinner = 0;
                } else if (++noWinner == autostop) {
                    irc_sendmsg(channel);
                    irc_sendformat(
                        true, "GameOver",
                        "[Game is over. Type !start to restart it.]");
                    cur_state = STOPPED;
                    time(&last_msg);
                    noWinner = 0;
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
                    scores->clear(Scoreboard::Type::Week);
                }
                lastDayOfWeek = systemTime->tm_wday;
            }

            // Announce bot banner
            if (systemTime->tm_hour != lastHour) {
                show_about();
                lastHour = systemTime->tm_hour;
            }

            while (irc_recv(line)) {
                pinger.recv();
                std::string nickname;
                std::string dummy;
                std::string cmd;
                std::string msg_channel;
                std::string paramtext;
                irc_analyze(std::move(line), nickname, cmd, msg_channel,
                            paramtext);
                if (cmd == "PRIVMSG" &&
                    strcasecmp(msg_channel.c_str(), channel.c_str()) == 0) {
                    paramtext = irc_stripcodes(paramtext);
                    if (paramtext.empty()) {
                        continue;
                    }
                    if (strncasecmp(paramtext.c_str(), "!r", 2) == 0) {
                        displayLetters(letters);
                    } else if (!scrabbleCmd(nickname.c_str(),
                                            paramtext.c_str())) {
                        if (paramtext.empty()) {
                            continue;
                        }
                        if (paramtext.size() > winningWordLen) {
                            non_ascii_strupr(
                                const_cast<char*>(paramtext.data()));
                            std::string sortedWord(paramtext);
                            std::sort(sortedWord.begin(), sortedWord.end());
                            if (isPossible(wordlen, sortedLetters,
                                           sortedWord) &&
                                isWord(dictionary, sortedWord, paramtext)) {
                                irc_sendmsg(channel);
                                winningWordLen = paramtext.size();
                                if (winningWordLen == foundWords.lenBestWords) {
                                    irc_sendformat(
                                        false, "Win",
                                        "Congratulations {:s} ! There isn't "
                                        "anything better [{:s}] !! You get "
                                        "{:d} points + {:d} bonus !",
                                        paramtext, nickname, winningWordLen,
                                        bonus);
                                    send_update_stats(nickname,
                                                      winningWordLen + bonus);
                                    if (autovoice) {
                                        irc_sendline(
                                            fmt::format("MODE {} +v {}",
                                                        channel, nickname));
                                    }
                                    tclock = 0;
                                    noWinner = 0;
                                } else {
                                    irc_sendformat(
                                        true, "Word",
                                        "Not bad {:s}... I'll keep your word "
                                        "[{:s}] !  Who can do better than {:d} "
                                        "letters ?",
                                        paramtext, nickname, winningWordLen);
                                }
                                winningNick = nickname;
                            }
                        }
                    }
                }
            }

            // Keep alive
            if (!pinger.is_alive()) {
                log("***** Timeout detected, reconnecting. *****");
                irc_disconnect_msg("Timeout detected, reconnecting.");
                return;
            }

        } while (cur_state == RUNNING && (tclock != 0u));
    }
}

void game_loop(Cell const* dictionary)
{
    while (cur_state != QUITTING) {

        // Connect to irc
        log("Connecting to IRC...");
        irc_connect();

        // Init execution
        show_about();
        run_game(dictionary);
    }

    scores->save();
    log("Quitting...");
    irc_disconnect();
}
