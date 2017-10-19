# lxScrabble

lxScrabble is an automated bot to play Scrabble in an IRC channel. lxScrabble is implemented as an autonomous program, not an addon, and therefore, does not require any external IRC client to be used. The program can be run in a terminal, and does not requires an X windows installation.

The goal of the game is to find the longest possible word that can be built from the letters suggested by the bot. The bot will validate any candidate words sent to the channel, and credit the user with as many points as letters the given word has. It will also track scores from players over different games.

lxScrabble is a *nix port of Wiz0u's WScrabble. Original WScrabble is available from Wiz0u's website: http://wiz0u.free.fr/wscrabble/index_en.php.

# Installation

1. Clone this repository:

``` bash
# git clone https://github.com/ricrogz/lxScrabble.git
```

2. Enter the directory and initialize the inicpp dependency:

``` bash
# cd lxScrabble
# git submodule update
```

3. Initialize the submodules inside inicpp (this is a requirement inherited from inicpp):

``` bash
# cd dependencies/inicpp
# git submodule update
```

4. Return to lxScrabble root directory, and compile. After this step, you should have a *lxScrabble* executable.

``` bash
# cd ../..
# mkdir build
# cd build
# cmake ..
# make
```

5. Copy the executable, the configuration file, and the dictionary file to the target directory:

```bash
# cp lxScrabble ../lxScrabble.ini ../twl98.txt [target directory]
```

6. Customize the configuration & start the game

```bash
# cd [target directory]
# vi lxScrabble.ini
# ./lxScrabble
```

# Configuration file options



# Changes from WScrabble

* Source code has been divided into several files for clarity.
* Windows library calls have been replaced have been replaced with POSIX equivalents.
* ini file handling is done through **inicpp**: https://github.com/SemaiCZE/inicpp.
* as a result of the usage of inicpp, code has been adapted to use *string* objects instead of *char[]* at certain points, and conserve functionalitiy.
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