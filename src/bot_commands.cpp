//
// Created by invik on 20/10/17.
//

#include "bot_commands.h"

#include "irc.h"

void help_cmd(const string &dest, bool is_owner) {

    string user_commands[] = {
            "!start: Start a game (works only when the bot is stopped).",
            "!score: Show your current score.",
            "!score <nick>: Show <nick>'s current score.",
            "!top10: Show this week's 10 best scores.",
            "!top: Alias of : !top10: .",
            "!top10year: Show this year's 10 best scores.",
            "!top3: Show this week's 3 best scores.",
            "!top3year: Show this year's 3 best scores.",
            "!r: Show again the letters for the ongoing game."
    };

    string stop_cmd = "!stop: Stop the bot.";

    string owner_commands[] = {
            "!newweek: Reset the weekly scores, and start a new week.",
            "!quit " + bot_nick + ": Quit. WARNING: this will stop the bot, and you will have to start the program again.",
            "!op: get op'ed by the bot (the bot must have 'op' itself)."
    };

    // User commands
    irc_sendmsg(dest);
    irc_sendline("User commands:");
    for (auto & cmd : user_commands) {
        irc_sendmsg(dest);
        irc_sendline(cmd);
    }
    if (anyoneCanStop) {
        irc_sendmsg(dest);
        irc_sendline(stop_cmd);
    }

    // Owner commands
    if (is_owner) {
        irc_sendmsg(dest);
        irc_sendline("Owner commands:");
        for (auto & cmd : owner_commands) {
            irc_sendmsg(dest);
            irc_sendline(cmd);
        }
        if (!anyoneCanStop) {
            irc_sendmsg(dest);
            irc_sendline(stop_cmd);
        }
    }
}