# lxScrabble

lxScrabble is an automated bot to play Scrabble in an IRC channel. lxScrabble is implemented as an autonomous program, not an addon, and therefore, does not require any external IRC client to be used. The program can be run in a terminal, and does not requires an X windows installation.

The goal of the game is to find the longest possible word that can be built from the letters suggested by the bot. The bot will validate any candidate words sent to the channel, and credit the user with as many points as letters the given word has. It will also track scores from players over different games.

lxScrabble is a *nix port of Wiz0u's WScrabble. Original WScrabble is available from Wiz0u's website: http://wiz0u.free.fr/wscrabble/index_en.php.

# Installation

1. Clone this repository:

``` bash
# git clone https://github.com/ricrogz/lxScrabble.git
```

1. Enter the directory and initialize the inicpp dependency:

``` bash
# cd lxScrabble
# git submodule update --recursive
```

1. Return to lxScrabble root directory, and compile. After this step, you should have a *lxScrabble* executable.

``` bash
# cd ..
# mkdir build
# cd build
# cmake ..
# make
```

1. Copy the executable, the configuration file, and the dictionary file to the target directory:

```bash
# cp lxScrabble ../lxScrabble.ini ../twl98.txt [target directory]
```

1. Customize the configuration & start the game

```bash
# cd [target directory]
# vi lxScrabble.ini
# ./lxScrabble
```

# Configuration file options

1. **Section \[IRC\]**
    2. **Server**: IRC server on which to play.
    2. **Port**: port on which **Server** listens to client connections.
    2. **Password**: password required by the IRC server to make a connection (usually ok to be left blank).
    2. **Nick**: Nick to be used by the bot in the IRC network.
    2. **ANick**: Alternate Nick to be used by the bot if **Nick** is already in use in the IRC network.
    2. **Ident**: IRC ident for the bot.
    2. **Fullname**: IRC fullname string for the bot.
    2. **Channel**: IRC channel to autojoin and where to play the game.
    2. **ChannelKey**: password used to join the **Channel**.
    2. **Owner**: comma-separated list of users allowed to give commands to the bot.
    2. **Perform**: Command to perform after successful connection to the IRC server.
    2. **BlackAndWhite**: Strip bot's messages of color codes.
    2. **AnyoneCanStop**: allow any user to stop the bot.

1. **Section \[Settings\]**
    2. **dictionary**: plain text file with the list of valid words that the game accepts.
    2. **wordlen**: number of letters to sample in each game (therefore, also the potential maximum length of words that can be built).
    2. **distribution**: complete set of letters from which to sample in each game.
    2. **autostop**: number of consecutive games with no winner before the bot stops.
    2. **bonus**: bonus points to award a player that finds the longest possible word that can be built in a game.

1. **Section \[Delay\]**
    2. **max**: maximum total duration, in seconds, of each game (the game may finish early if any player finds a word of the maximum possible length).
    2. **warning**: Show a warning about time running out this time after a game's start.
    2. **after**: time to wait after a game finishes to start next one.

1. **Section \[Strings\]**
The options and strings in this section enumerate the format strings that will be used by the program to signal different events. Customize these strings i.e. to translate the game into other languages. Please respect these two rules, or the game may not work properly:
    2. Escape any commas, colons or semicolons ('\\,', '\\:', '\\;').
    2. Respect the formatters ('%d' or '%s'): you can alter the position of the formatters in the string, but please respect their order and number (do not add or remove any formatters).

# Changes from WScrabble

* Source code has been divided into several files for clarity.
* Windows library calls have been replaced have been replaced with POSIX equivalents.
* ini file handling is done through **inicpp**: https://github.com/SemaiCZE/inicpp.
* as a result of the usage of inicpp, code has been adapted to use *string* objects instead of *char\[\]* at certain points, and conserve functionalitiy.
* **also as a result of using inicpp** the format of ini files has changed slightly. Specifically:
    - Format strings in lxScrabble.ini are no longer embedded in quotes.
    - Commas, colons and semicolons (',', ':', ';') in format strings in lxScrabble need to be escaped ('\\,', '\\:', '\\;').
    - Formatting placeholders ('%...') format strings in lxScrabble have been replaced by proper *printf* placeholders ('%s', '%d).
    - Player scores in score.ini do no longer start with an underscore ('_').

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