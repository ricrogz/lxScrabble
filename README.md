# lxScrabble

lxScrabble is an automated bot to play Scrabble in an IRC channel. It is implemented as an autonomous program, not an add-on, and therefore, does not require any external IRC client to be used. The program can be run in a terminal, and does not require an X windows installation.

The goal of the game is to find the longest possible word that can be built from the letters suggested by the bot. The bot will validate any candidate words sent to the channel, and credit the user with as many points as letters the given word has. It will also track scores from players over different games.

lxScrabble is a linux port (although it probably can be used in any \*nix system) of Wiz0u's WScrabble. Original WScrabble is available from Wiz0u's website: http://wiz0u.free.fr/wscrabble/index_en.php.

**Disclaimer #1**: I am not a C nor C++ programmer, and the port has been done following a *minimum effort* strategy, so please do not understand any of my code as a "best practice"; not even as a "good practice".

**Disclaimer #2**: This is a **development version**. This means that the program has not been thoroughly tested, and therefore, it may contain bugs which might cause problems to your system. By using this software, you acknowledge this warning and agree that I will not be held liable for any consequences of its usage.

# Installation

1. Clone this repository:

    ``` bash
    $ git clone https://github.com/ricrogz/lxScrabble.git
    ```

1. Enter the directory and initialize the inicpp dependency:

    ``` bash
    $ cd lxScrabble
    $ git submodule init
    $ git submodule update --recursive
    ```

    Depending on your linux distro/version, you might also need to do:

    ``` bash
    $ cd dependencies/inicpp
    $ git submodule init
    $ git submodule update
    $ cd ../..
    ```
    
1. Return to lxScrabble root directory, and compile. After this step, you should have a *lxScrabble* executable.

    ``` bash
    $ cd ..
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    ```

1. Copy the executable, the configuration file, and the dictionary file to the target directory:

    ```bash
    $ cp lxScrabble ../lxScrabble.ini ../twl98.txt <target directory>
    ```

1. Customize the configuration & start the program. Once the program is started, it will attempt to read the configuration file, connect to the specified IRC server, and automatically join the specified channel and start the game.

    ```bash
    $ cd <target directory>
    $ vi lxScrabble.ini
    $ ./lxScrabble
    ```

To stop the bot, either issue "!quit <bot's nick>" with an "Owner" nick, or interrupt it with Ctrl-c from the console where it runs.

# Configuration file options

Any of the options, except **Server**, can be disabled by putting a semicolon (';') in front of it. The bot will then use a default value for it.

1. **Section \[IRC\]**
    * **Server**: IRC server on which to play.
    * **Port**: port on which **Server** listens to client connections.
    * **Password**: password required by the IRC server to make a connection (usually ok to be left blank).
    * **Nick**: Nick to be used by the bot in the IRC network.
    * **ANick**: Alternate Nick to be used by the bot if **Nick** is already in use in the IRC network.
    * **Ident**: IRC ident for the bot.
    * **Fullname**: IRC fullname string for the bot.
    * **Channel**: IRC channel to autojoin and where to play the game.
    * **ChannelKey**: password used to join the **Channel**.
    * **Owner**: comma-separated list of users allowed to give commands to the bot.
    * **Perform**: Commands (lines separated with '|') to perform after successful connection to the IRC server.
    * **BlackAndWhite**: Strip bot's messages of color codes.
    * **AnyoneCanStop**: allow any user to stop the bot.

1. **Section \[Settings\]**
    * **dictionary**: plain text file with the list of valid words that the game accepts.
    * **wordlen**: number of letters to sample in each game (therefore, also the potential maximum length of words that can be built).
    * **distribution**: complete set of letters from which to sample in each game. Currently, only ASCII characters may be used.
    * **autostop**: number of consecutive games with no winner before the bot stops.
    * **bonus**: bonus points to award a player that finds the longest possible word that can be built in a game.

1. **Section \[Delay\]**
    * **max**: maximum total duration, in seconds, of each game (the game may finish early if any player finds a word of the maximum possible length).
    * **warning**: Show a warning about time running out this time after a game's start.
    * **after**: time to wait after a game finishes to start next one.

1. **Section \[Strings\]**
The options and strings in this section enumerate the format strings that will be used by the program to signal different events. Customize these strings i.e. to translate the game into other languages. Please respect these two rules, or the game may not work properly:

    * Escape any commas, colons or semicolons ('\\,', '\\:', '\\;').
    * Respect the formatters ('%d' or '%s'): you can alter the position of the formatters in the string, but please respect their order and number (do not add or remove any formatters).

# Remote commands

## Comands usable by any user

Channel messages recognized by the bot as commands:

* **!help**: get the list of available commands in a private query.
* **!start**: Start a game (works only when the bot is stopped).
* **!score**: Show your current score.
* **!score <nick>**: Show <nick>'s current score.
* **!top10**: Show this week's 10 best scores.
* **!top**: Alias of **!top10**.
* **!top10year**: Show this year's 10 best scores.
* **!top3**: Show this week's 3 best scores.
* **!top3year**: Show this year's 3 best scores.
* **!r**: Show again the letters for the ongoing game.

## Owner commands

These commands may only be used by users whose nick is listed in the **Owner** config option.

* **!stop**: Stop the bot.
* **!newweek**: Reset the weekly scores, and start a new week.
* **!quit <bot's nick>**: Quit. **WARNING**: this will stop the bot, and you will have to execute the program again.
* **!op**: get op'ed by the bot (the bot must have 'op' itself).

# Usage of non-ascii characters

Before non-ascii characters can be used, they have to be incorporated to the code of the program. The proper way of doing it is in the header file "mimics.h", by adding them to the defines `NONLATIN_LCASE` and `NONLATIN_UCASE`, respetively for the lowercase character and its uppercase equivalent. Please keep the characters ordered so that different cases of the same letters are placed at the same indexes in both strings!

# Changes from WScrabble

* Source code has been divided into several files for clarity.
* Windows library calls have been replaced have been replaced with POSIX equivalents.
* ini file handling is done through **inicpp**: https://github.com/SemaiCZE/inicpp.
* as a result of the usage of inicpp, code has been adapted to use *string* objects instead of *char\[\]* at certain points, and conserve functionality.
* **also as a result of using inicpp** the format of ini files has changed slightly. Specifically:
    - Format strings in lxScrabble.ini are no longer embedded in quotes.
    - Commas, colons and semicolons (',', ':', ';') in format strings in lxScrabble need to be escaped ('\\,', '\\:', '\\;').
    - Formatting placeholders ('%...') format strings in lxScrabble have been replaced by proper *printf* placeholders ('%s', '%d').
    - Player scores in score.ini do no longer start with an underscore ('_').
* Current source code allows for incorporation of non-ascii characters.

# Potential future improvements

* Mute/redirect output to a log file.
* Improved score tracking functionality, plus usage of a SQLite database to track scores.
* Mutichannel capability.
* Dictionary selection between different options.
* Global code rewrite.

# License

lxScrabble and WScrabble are licensed under the Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0, https://creativecommons.org/licenses/by-nc-sa/4.0/).

This means that you can:

* Share — copy and redistribute the material in any medium or format.
* Adapt — remix, transform, and build upon the material.

Under the following terms:

* Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
* NonCommercial — You may not use the material for commercial purposes.
* ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.
* No additional restrictions — You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.
