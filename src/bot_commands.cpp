//
// Created by invik on 20/10/17.
//
#include <vector>

#include "bot_commands.hpp"

#include "irc.hpp"

const std::vector<std::string> user_commands = {
    "\002!start\002: Start a game (works only when the bot is stopped).",
    "\002!score\002: Show your current score.",
    "\002!score <nick>\002: Show <nick>'s current score.",
    "\002!top10\002: Show this week's 10 best scores.",
    "\002!top\002: Alias of \002!top10\002.",
    "\002!top10total\002: Show all times's 10 best scores.",
    "\002!top3\002: Show this week's 3 best scores.",
    "\002!top3total\002: Show all time's 3 best scores.",
    "\002!r\002: Show again the letters for the ongoing game."};

const std::string stop_cmd = "\002!stop\002: Stop the bot.";

const std::vector<std::string> owner_commands = {
    "\002!newweek\002: Reset the weekly scores, and start a new week.",
    "\002!quit [bot_nick]\002: Quit. WARNING: this will stop the bot, and you "
    "will have to start the program again.",
    "\002!op\002: get op'ed by the bot (the bot must have 'op' itself)."};

void help_cmd(const std::string& dest, bool is_owner)
{
    // User commands
    irc_sendmsg(dest);
    irc_sendline("User commands:");
    for (const auto& cmd : user_commands) {
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
        for (const auto& cmd : owner_commands) {
            irc_sendmsg(dest);
            irc_sendline(cmd);
        }
        if (!anyoneCanStop) {
            irc_sendmsg(dest);
            irc_sendline(stop_cmd);
        }
    }
}