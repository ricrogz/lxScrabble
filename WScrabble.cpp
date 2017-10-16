// This work is licensed under the CC BY-NC-SA 4.0 license (Creative Common Attribution-NonCommercial-ShareAlike 4.0 International). See LICENSE.txt
// ______________________________________________________________
// WScrabble.cpp : définit le point d'entrée pour l'application console.
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>

#include "port.h"

/* Windows includes */
// #include <tchar.h>
// #include <conio.h>
// #include <winsock.h>


#if defined(_DEBUG) && 0
#define OFFLINE
#define CHEAT
#endif

#define BOTNAME "WScrabble 1.10" // penser a mettre à jour VERSIONINFO aussi
#define BOTFULLNAME BOTNAME " by Wizou"
#define ADVERTISE "\x002""\x003""9,2 ~\x003""08  \x003""04¤ \x003""00" BOTNAME " \x003""15-\x003""00 http://wiz0u.free.fr/wscrabble \x003""04¤\x003""08  \x003""09~\x00F"""


using namespace std;

int halt_status = -1;

BOOL WINAPI CtrlHandlerRoutine(DWORD dwCtrlType)
{
	dwCtrlType;
	exit(halt_status);
}

void halt(int status)
{
	halt_status = status;
	SetConsoleCtrlHandler(CtrlHandlerRoutine, TRUE);
	SetConsoleTitle("<program terminated>");
	getch();
	exit(status);
}

#define NICK_MAX 40

SOCKET irc_socket;
char irc_buffer[8193];
size_t irc_bufLen = 0;
BOOL irc_blackAndWhite = FALSE;

void irc_stripcodes(char *text)
{
	char *scan = text;
	unsigned char ch;
	while ((ch = *scan++) != 0)
	{
		if (ch == 3) // couleur CTRL-K
		{
			if (isdigit(*scan)) // suivi d'un chiffre
			{
				if (isdigit(*++scan)) scan++; // eventuellement un 2eme chiffre
				if ((*scan == ',') && isdigit(*(scan+1))) // eventellement une virgule suivie d'un chiffre
				{
					scan += 2;
					if (isdigit(*scan)) scan++; // eventuellement un 2eme chiffre
				}
			}
		}
		else if (ch >= 32)
			*text++ = ch;
	}
	*text++ = 0;
}

void irc_init(void)
{
#ifndef OFFLINE
	irc_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
/*	LINGER linger;
	linger.l_onoff = TRUE;
	linger.l_linger = 2;
	setsockopt(irc_socket, SOL_SOCKET, SO_LINGER, (const char*) &linger, sizeof(linger));*/
#endif
}

void irc_sendformatline(LPCTSTR lpKeyName, LPCTSTR lpDefault, ...)
{
	char buffer[1024];
	LPTSTR text;
	va_list arguments;
	va_start(arguments, lpDefault);
	GetPrivateProfileString("Strings", lpKeyName, lpDefault, buffer, sizeof(buffer), "./WScrabble.ini");
	if (irc_blackAndWhite) irc_stripcodes(buffer);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING, buffer, 0, 0, (LPTSTR) &text, 0, &arguments);
	va_end(arguments);
	cout << text << endl;
#ifndef OFFLINE
	send(irc_socket, text, (int) strlen(text), 0);
	send(irc_socket, "\n", 1, 0);
#endif
	LocalFree(text);
}

void irc_sendformat(LPCTSTR lpKeyName, LPCTSTR lpDefault, ...)
{
	char buffer[1024];
	LPTSTR text;
	va_list arguments;
	va_start(arguments, lpDefault);
	GetPrivateProfileString("Strings", lpKeyName, lpDefault, buffer, sizeof(buffer), "./WScrabble.ini");
	if (irc_blackAndWhite) irc_stripcodes(buffer);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING, buffer, 0, 0, (LPTSTR) &text, 0, &arguments);
	va_end(arguments);
	cout << text;
#ifndef OFFLINE
	send(irc_socket, text, (int) strlen(text), 0);
#endif
	LocalFree(text);
}

void irc_send(const char *text)
{
	cout << text;
#ifndef OFFLINE
	send(irc_socket, text, (int) strlen(text), 0);
#endif
}
void irc_send(char ch)
{
	cout << ch;
#ifndef OFFLINE
	send(irc_socket, &ch, 1, 0);
#endif
}
void irc_send(int value)
{
	cout << value;
	char text[32];
	itoa(value, text, 10);
#ifndef OFFLINE
	send(irc_socket, text, (int) strlen(text), 0);
#endif
}
void irc_sendline(const char *line)
{
	cout << line << endl;
#ifndef OFFLINE
	send(irc_socket, line, (int) strlen(line), 0);
	send(irc_socket, "\n", 1, 0);
#endif
}
bool irc_handlestd(char line[1024])
{
	if (strncmp(line, "PING :", 6) == 0)
	{
		irc_send("PONG :");
		irc_sendline(line+6);
		return true;
	}
	return false;
}
bool irc_recv(char line[1024])
{
	line;
#ifndef OFFLINE
	u_long value = 0;
	ioctlsocket(irc_socket, FIONREAD, &value);
	if (value)
	{
		irc_bufLen += recv(irc_socket, irc_buffer+irc_bufLen, min(value, sizeof(irc_buffer)-irc_bufLen), 0);
		irc_buffer[irc_bufLen] = 0;
		irc_bufLen = strlen(irc_buffer);
	}
	for (;;)
	{
		char *scan = strchr(irc_buffer, '\n');
		if (scan)
		{
			int lineLen = (int) (scan-irc_buffer);
			if (lineLen > 1024) lineLen = 1024;
			strncpy(line, irc_buffer, lineLen-1);
			line[lineLen-1] = 0;
			irc_bufLen -= lineLen+1;
			memmove(irc_buffer, irc_buffer+lineLen+1, irc_bufLen+1);
			cout << "<- " << line << endl;
			if (!irc_handlestd(line))
				return true;
		}
		else
			return false;
	}
#else
	return false;
#endif
}

void irc_analyze(char *line, char **nickname, char **ident, char **hostname, char **cmd, char **param1, char **param2, char **paramtext)
{
	char *scan;
	*nickname = *ident = *hostname = NULL;
	if (line[0] == ':')
	{
		scan = strchr(line, ' ');
		*scan = 0;
		*cmd = scan+1;
		*nickname = line+1;
		scan = strchr(line+1, '!');
		if (scan)
		{
			*scan++ = 0;
			*ident = scan;
			scan = strchr(scan, '@');
			*scan++ = 0;
			*hostname = scan;
		}
	}
	else
		*cmd = line;
	scan = strchr(*cmd, ' ');
	*scan++ = 0;
	*param1 = *param2 = NULL;
	*paramtext = strchr(scan,'\0');
	while (*scan != ':')
	{
		if (!*param1) *param1 = scan;
		else if (!*param2) *param2 = scan;
		else
		{
			scan--;
			break;
		}
		scan = strchr(scan, ' ');
		if (scan == NULL)
			return;
		*scan++ = 0;
	}
	*paramtext = scan+1;
}

bool irc_want(const char *wantCmd, UINT timeout = 15000)
{
#ifdef OFFLINE
	wantCmd, timeout;
	return true;
#else
	char line[1024];
	DWORD ticks = GetTickCount();
	while (GetTickCount()-ticks < timeout)
	{
		if (irc_recv(line))
		{
			char *cmd, *dummy;
			irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy, &dummy);
			if (strcmp(cmd, wantCmd) == 0)
				return true;
		}
		else
			Sleep(100);
	}
	return false;
#endif
}

void irc_flushrecv()
{
	char line[1024];
	do
	{
		while (irc_recv(line));
		Sleep(500);
	} while (irc_recv(line));
}

void irc_connect(const char *servername, int port, const char *password, const char *nickname, const char *altnickname, const char *ident, const char *localhost, const char *fullname)
{
#ifdef OFFLINE
	servername, port, password, nickname, altnickname, ident, localhost, fullname;
#else
	hostent *host = gethostbyname(servername);
	if (!host)
	{
		cerr << "Could not resolve server name" << endl;
		halt(2);
	}
	sockaddr_in name;
	name.sin_addr = *(struct in_addr*) host->h_addr;
	name.sin_family = host->h_addrtype;
	name.sin_port = htons((u_short) port);
	connect(irc_socket, (sockaddr*) &name, sizeof(name));

	Sleep(500);
	irc_flushrecv();

	if (password && password[0])
	{
		irc_send("PASS ");
		irc_sendline(password);
	}

	irc_send("NICK ");
	irc_sendline(nickname);

	irc_send("USER ");
	irc_send(ident);
	irc_send(' ');
	irc_send(localhost);
	irc_send(' ');
	irc_send(servername);
	irc_send(" :");
	irc_sendline(fullname);

	char line[1024];
	DWORD ticks = GetTickCount();
	while (GetTickCount()-ticks < 45000)
	{
		if (irc_recv(line))
		{
			char *cmd, *dummy;
			irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy, &dummy);
			if (strcmp(cmd,"001") == 0)
			{
				irc_flushrecv();
				return;
			}
			else if ((strcmp(cmd,"432") == 0) || (strcmp(cmd,"433") == 0))
			{
				if (strcmp(nickname, altnickname) == 0)
				{
					cerr << "Nicknames already in use" << endl;
					halt(6);
				}
				nickname = altnickname;
				irc_send("NICK ");
				irc_sendline(nickname);
			}
		}
		else
			Sleep(500);
	}
	cerr << "Problem during connection" << endl;
	halt(7);
#endif
}
void irc_join(const char *channel, const char *channelkey = NULL)
{
	irc_send("JOIN ");
	if (channelkey && channelkey[0])
	{
		irc_send(channel);
		irc_send(" ");
		irc_sendline(channelkey);
	}
	else
		irc_sendline(channel);
	irc_want("JOIN");
}
void irc_sendmsg(const char *dest)
{
	irc_send("PRIVMSG ");
	irc_send(dest);
	irc_send(" :");
}
void irc_sendnotice(const char *dest)
{
	irc_send("NOTICE ");
	irc_send(dest);
	irc_send(" :");
}

void irc_end(void)
{
	irc_sendline("QUIT :Game Over");
#ifndef OFFLINE
	closesocket(irc_socket);
#endif
}

#define DISTRIB_MAX 200
char distrib[DISTRIB_MAX];
size_t wordlen;

struct Cell {
	struct Cell *other; // autres lettres possibles (plus grandes dans l'ordre alphabétiques)
	struct Cell *longer; // mots plus long disponibles
	char *words; // succession des mots contenant ces lettres, séparés par des \0 et terminé par un autre \0
	unsigned short wordsCount;
	char letter; // la lettre
};

#define WORD_MAX 64

struct Top {
	char nick[NICK_MAX];
	int score;
};

#define TOP_MAX 10

struct Cell *dictionnary = NULL;
char channel[64];
char channelkey[64];
char owner[1024];
BOOL anyoneCanStop;
int bonus = 10;
int foundWords;
int foundMaxWords;
int maxWordLen;
int dispMaxWords;
char dispMaxWordsString[1024];
char lastWinner[NICK_MAX];
struct Top topWeek[TOP_MAX];
struct Top topYear[TOP_MAX];

int winInARow;
enum { running, stopped, halting } cur_state;
	
void sortLetters(const char *letters, char sortedLetters[WORD_MAX])
{
	char *scan = sortedLetters;
	char ch;
	sortedLetters[0] = 127;
	while ((ch = *letters++) != '\0')
	{
		scan = sortedLetters;
		while (ch >= *scan) scan++;
		char ch2;
		do
		{
			ch2 = *scan;
			*scan++ = ch;
		} while ((ch = ch2) != 127);
		*scan = 127;
	}
	*scan = 0;
}

void addWord(const char *word)
{
	char letters[WORD_MAX];
	sortLetters(word, letters);
    struct Cell **cell = &dictionnary;
	char *scan = letters;
	char ch = *scan;
	int len = 1;
	while (ch)
	{
		len++;
		for (;;)
		{
			if ((*cell == NULL) || ((*cell)->letter > ch)) {
				struct Cell *newcell = (struct Cell*) malloc(sizeof(struct Cell));
				newcell->other = *cell;
				newcell->longer = NULL;
				newcell->words = NULL;
				newcell->wordsCount = 0;
				newcell->letter = ch;
				*cell = newcell;
				break;
			}
			else if ((*cell)->letter == ch) break;
			cell = &(*cell)->other;
		}
		if ((ch = *++scan) != '\0')
			cell = &(*cell)->longer;
	}
	int wordsCount = (*cell)->wordsCount++;
	(*cell)->words = (char*) realloc((*cell)->words,(wordsCount+1)*len);
	strcpy(&(*cell)->words[wordsCount*len], word);
}

void readDictionnary(const char *filename)
{
	ifstream stream(filename);
	if (stream.fail())
	{
		cerr << "Could not open dictionnary file" << endl;
		halt(3);
	}
	char word[WORD_MAX];
	while (!stream.eof())
	{
		stream >> word;
		size_t len = strlen(word);
		if (len <= 0) continue;
		if (len > wordlen) continue;
		if (strspn(word, distrib) != len)
		{
			cerr << "Invalid dictionnary entry: " << word << endl;
			cerr << "(contains lowercase letters or symbols not in the valid distribution)" << endl;
			halt(3);
		}
		strupr(word);
		addWord(word);
	}
}


void findWords(const struct Cell *cell, const char *letters, int len)
{
	char ch = *letters++;
	while (cell && (cell->letter < ch)) cell = cell->other;
	if (cell)
	{
		if (cell->letter == ch) 
		{
			if (cell->wordsCount)
			{
				if (dispMaxWords)
				{
					if (len == dispMaxWords)
					{
						for (int index = 0; index < cell->wordsCount; index++)
						{
							strcat(dispMaxWordsString, " - ");
							strcat(dispMaxWordsString, cell->words+(len+1)*index);
						}
					}
				}
				else
				{
					foundWords += cell->wordsCount;
					if (len > maxWordLen)
					{
						foundMaxWords = cell->wordsCount;
						maxWordLen = len;
					}
					else if (len == maxWordLen)
						foundMaxWords += cell->wordsCount;
				}
			}
			if ((*letters != 0) && cell->longer)
			{
				findWords(cell->longer, letters,len+1);
			}
			while (*letters == ch) letters++;
		}
		if (*letters != 0)
			findWords(cell,letters,len);
	}
}

void findWords(const char letters[WORD_MAX])
{
	dispMaxWords = 0;
	foundWords = 0;
	foundMaxWords = 0;
	maxWordLen = 0;
	findWords(dictionnary, letters, 1);
}

void displayMaxWords(const char letters[WORD_MAX], int len)
{
	dispMaxWordsString[0] = '\0';
	dispMaxWords = len;
	foundWords = 0;
	foundMaxWords = 0;
	maxWordLen = 0;
	findWords(dictionnary, letters, 1);
}

bool isWord(const char *letters, const char *word)
{
	char ch = *letters++;
	const struct Cell *cell = dictionnary;
	do
	{
		while (cell && (cell->letter < ch)) cell = cell->other;
		if (!cell) return false;
		if (cell->letter == ch)
		{
			ch = *letters++;
			if (ch == 0)
			{
				size_t len = strlen(word)+1;
				for (int index = 0; index < cell->wordsCount; index++)
					if (strcmp(cell->words+index*len,word) == 0)
						return true;
				return false;
			}
			else
				cell = cell->longer;
		}
		else
			return false;
	} while (cell);
	return false;
}

bool checkWord(const char availableLetters[WORD_MAX], const char *word)
{
	char letters[WORD_MAX];
	size_t len = strlen(word);
	if (len > wordlen) return false;
	sortLetters(word, letters);
	int scan = wordlen;
	while (len--)
	{
		do
		{
			if (scan == 0) return false;
		} while (availableLetters[--scan] > letters[len]);
		if (availableLetters[scan] != letters[len]) return false;
	}
	return isWord(letters, word);
}

void clear_top(Top *top)
{
	for (int index = 0; index < TOP_MAX; index++)
	{
		top[index].score = 0;
		strcpy(top[index].nick, "---");
	}
}

void read_top(Top *top, char *value)
{
	char *scan = value;
	clear_top(top);
	strcat(value, " ");
	do
	{
		scan = strchr(value, ':');
		if (!scan) break;
		*scan = '\0';
		strcpy(top->nick, value);
		value = scan+1;
		scan = strchr(value, ' ');
		*scan = '\0';
		top->score = atoi(value);
		value = scan+1;
		top++;
	} while (*value);
}

void write_top(Top *top, char *value)
{
	for (int index = 0; index < TOP_MAX; index++)
	{
		if (top->score == 0) break;
		strcpy(value, top->nick);
		value += strlen(value);
		*value++ = ':';
		itoa(top->score, value, 10);
		value += strlen(value);
		*value++ = ' ';
		top++;
	}
	if (index) --value;
	*value = '\0';
}

void read_tops(void)
{
	char value[(NICK_MAX+10)*TOP_MAX];
	GetPrivateProfileString("Top", "Week", "", value, sizeof(value), ".\\scores.ini");
	read_top(topWeek, value);
	GetPrivateProfileString("Top", "Year", "", value, sizeof(value), ".\\scores.ini");
	read_top(topYear, value);
}

void write_tops(void)
{
	char value[(NICK_MAX+10)*TOP_MAX];
	write_top(topWeek, value);
	WritePrivateProfileString("Top", "Week", value, ".\\scores.ini");
	write_top(topYear, value);
	WritePrivateProfileString("Top", "Year", value, ".\\scores.ini");
}

bool update_top(Top *top, const char *nickname, int score)
{
	int newPos;
	for (newPos = 0; newPos < TOP_MAX; newPos++)
		if (score >= top[newPos].score)
			break;
	if (newPos == TOP_MAX) return false; // le score n'entre pas dans le top
	for (int index = newPos; index < TOP_MAX-1; index++)
		if (stricmp(top[index].nick, nickname) == 0)
			break; // le nom etait déjà dans le top
	// 0 1 2 3 4 5 6 7 8 9
	memmove(&top[newPos+1], &top[newPos], (index-newPos)*sizeof(Top));
	strcpy(top[newPos].nick, nickname);
	top[newPos].score = score;
	return true;
}

void clear_week_scores(void)
{
	char *buffer = (char*) malloc(32767);
	GetPrivateProfileSection("Scores", buffer, 32767, ".\\scores.ini");
	char *scan = buffer;
	while (*scan)
	{
		scan = strchr(scan, '=')+1;
		scan = strchr(scan, ' ')+1;
		*scan++ = '0';
		while (isdigit(*scan))
			*scan++ = ' ';
		scan = strchr(scan, '\0')+1;
	}
	WritePrivateProfileSection("Scores", buffer, ".\\scores.ini");
	free(buffer);
	clear_top(topWeek);
	write_tops();
}

void get_scores(const char *nickname, int *year, int *week)
{
	char value[50], *scan;
	char lnickname[NICK_MAX+1];
	lnickname[0] = '_';
	strcpy(lnickname+1, nickname);
	GetPrivateProfileString("Scores", lnickname, "0 0", value, sizeof(value), ".\\scores.ini");
	*year = strtoul(value, &scan, 10);
	*week = strtoul(scan, &scan, 10);
}

void set_scores(const char *nickname, int year, int week)
{
	char value[50];
	char lnickname[NICK_MAX+1];
	lnickname[0] = '_';
	strcpy(lnickname+1, nickname);
	sprintf(value, "%d %d", year, week);
	WritePrivateProfileString("Scores", lnickname, value, ".\\scores.ini");
	bool updated = update_top(topWeek, nickname, week);
	updated |= update_top(topYear, nickname, year);
	if (updated) write_tops();
}

void send_update_stats(const char *nickname, int gain)
{
	int year, week;
	get_scores(nickname, &year, &week);
	year+=gain; week+=gain;
	set_scores(nickname, year, week);

	if (stricmp(lastWinner, nickname) != 0)
	{
		winInARow = 1;
		irc_sendformatline("Stats", "( %1!d! pts :  %2!d! pts)", week, year);
	}
	else
	{
		winInARow++;
		irc_sendformatline("StatsCont", "( %1!d! pts :  %2!d! pts) - %3!d!  contiguous won games!", week, year, winInARow);
	}
	strcpy(lastWinner, nickname);
}

void displayLetters(const char letters[WORD_MAX])
{
	irc_sendmsg(channel);
	char s_letters[WORD_MAX*2];
	for (size_t i = 0; i < wordlen; i++)
	{
		s_letters[i*2] = letters[i];
		s_letters[i*2+1] = ' ';
	}
	s_letters[wordlen*2-1] = '\0';
	irc_sendformatline("Letters", "[Mixed Letters]  - %1 -", s_letters);
}

void replyScore(const char *nickname, const char *dest)
{
	int year, week;
	get_scores(nickname, &year, &week);
	irc_sendnotice(dest);
	if (year == 0)
		irc_sendformatline("ScoreUnknown", "%1 has never played with me.", nickname);
	else
		irc_sendformatline("Score", "%1's score is %2!d! point(s) for this week, %3!d! for this year.", nickname, week, year);
}

void replyTop10(const char *dest, Top *top, const char *whichTop, const char *lpDefault, const char *whichTopMore, const char *lpDefaultMore)
{
	irc_sendnotice(dest);
	irc_sendformatline(whichTop, lpDefault, top[0].nick, top[0].score);
	irc_sendnotice(dest);
	irc_sendformatline(whichTopMore, lpDefaultMore, 2, top[1].nick, top[1].score, 3, top[2].nick, top[2].score, 4, top[3].nick, top[3].score);
	irc_sendnotice(dest);
	irc_sendformatline(whichTopMore, lpDefaultMore, 5, top[4].nick, top[4].score, 6, top[5].nick, top[5].score, 7, top[6].nick, top[6].score);
	irc_sendnotice(dest);
	irc_sendformatline(whichTopMore, lpDefaultMore, 8, top[7].nick, top[7].score, 9, top[8].nick, top[8].score, 10,top[9].nick, top[9].score);
}

void replyTop3(const char *dest, Top *top, const char *whichTop, const char *lpDefault)
{
	irc_sendnotice(dest);
	irc_sendformatline(whichTop, lpDefault, top[0].nick, top[0].score, top[1].nick, top[1].score, top[2].nick, top[2].score);
}

void show_about(void)
{
	char buffer[sizeof(ADVERTISE)];
	strcpy(buffer, ADVERTISE);
	irc_sendmsg(channel);
	if (irc_blackAndWhite) irc_stripcodes(buffer);
	irc_sendline(buffer);
}

bool is_owner(const char *nickname)
{
	const char *scan = StrStrI(owner, nickname);
	if (scan != NULL)
	{
		if ((scan == owner) || (scan[-1]==','))
		{
			scan += strlen(nickname);
			if ((*scan == '\0') || (*scan == ','))
				return true;
		}
	}
	return false;
}

bool scrabbleCmd(const char *nickname, char *command)
{
	bool isOwner = is_owner(nickname);
	if (stricmp(command, "!score") == 0)
		replyScore(nickname, nickname);
	else if (strnicmp(command, "!score ", 7) == 0)
		replyScore(command+7, nickname);
	else if ((stricmp(command, "!top") == 0) || (stricmp(command, "!top10") == 0))
		replyTop10(nickname, topWeek, 
			"TopW10",		"The 10 best players of the week: 1. %1 (%2!d!)",
			"TopW10More",	"%1!d!. %2 (%3!d!) - %4!d!. %5 (%6!d!) - %7!d!. %8 (%9!d!)");
	else if (stricmp(command, "!top10year") == 0)
		replyTop10(nickname, topYear, 
			"TopY10",		"The 10 best players of the year: 1. %1 (%2!d!)",
			"TopY10More",	"%1!d!. %2 (%3!d!) - %4!d!. %5 (%6!d!) - %7!d!. %8 (%9!d!)");
	else if (stricmp(command, "!top3") == 0)
		replyTop3(nickname, topWeek, "TopW3", "The 3 best players of the week:  1. %1 (%2!d!) - 2. %3 (%4!d!) - 3. %5 (%6!d!)");
	else if (stricmp(command, "!top3year") == 0)
		replyTop3(nickname, topYear, "TopY3", "The 3 best players of the year:  1. %1 (%2!d!) - 2. %3 (%4!d!) - 3. %5 (%6!d!)");
	else if (stricmp(command, "!start") == 0)
		cur_state = running;
	else if ((anyoneCanStop || isOwner) && (stricmp(command, "!stop") == 0))
	{
		if (cur_state == running)
		{
			irc_sendmsg(channel);
			irc_sendformatline("Stop", "%1 has stopped the game.", nickname);
		}
		cur_state = stopped;
	}
	else if (isOwner)
	{
		if (stricmp(command, "!newweek") == 0)
		{
			irc_sendmsg(channel);
			irc_sendformatline("NewWeek", "A new week is beginning ! Resetting all week scores...");
			clear_week_scores();
		}
		else if (stricmp(command, "!disconnect") == 0)
		{
			cur_state = halting;
		}
		else if (stricmp(command, "!op") == 0)
		{
			irc_send("MODE ");
			irc_send(channel);
			irc_send(" +o ");
			irc_sendline(nickname);
		}
		else
			return false;
	}
	else
		return false;
	return true; // commande reconnue
}

void pickLetters(char letters[WORD_MAX], char sortedLetters[WORD_MAX])
{
	int count = strlen(distrib);
	char availableLetters[DISTRIB_MAX];
	strcpy(availableLetters, distrib);
	for (size_t index = 0; index < wordlen; index++)
	{
		unsigned long value;
		do {
			value = ((unsigned long) rand())*count/RAND_MAX;
		} while (availableLetters[value] == 0);
		letters[index] = availableLetters[value];
		availableLetters[value] = 0;
	}
	letters[wordlen] = 0;
#ifdef CHEAT
	cin >> letters;
#endif
	sortLetters(letters, sortedLetters);
}

void do_perform(char *perform)
{
	char *token = strtok(perform, "|");
	while (token != NULL)
	{
		token = token+strspn(token, " ");
		if (*token == '/') token++;
		char *scan = strchr(token, ' ');
		if (scan) *scan++ = '\0';
		strupr(token);
		if ((strcmp(token, "MSG") == 0) || (strcmp(token, "NOTICE") == 0))
		{
			if (!scan)
			{
				cerr << "Missing argument for " << token << " on Perform= setting" << endl;
				halt(2);
			}
			if (strcmp(token, "MSG") == 0)
				irc_send("PRIVMSG");
			else
				irc_send(token);
			token = scan;
			scan = strchr(token, ' ');
			*--token = ' ';
			*scan++ = '\0';
			irc_send(token);
			irc_send(" :");
			irc_sendline(scan);
		}
		else
		{
			if (scan) *--scan = ' ';
			irc_sendline(token);
		}
		token = strtok( NULL, "|");
	}

}

int _tmain(int argc, _TCHAR* argv[])
{
	argc; argv;
	cout << BOTFULLNAME << endl;
	srand( (unsigned)time( NULL ) ); rand();
	{
		char filename[128];
		cout << "Loading dictionnary..." << endl;
		wordlen = GetPrivateProfileInt("Settings", "wordlen",   10, "./WScrabble.ini");
		bonus = GetPrivateProfileInt("Settings", "bonus",   10, "./WScrabble.ini");
		GetPrivateProfileString("Settings", "distribution",   "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ", distrib, sizeof(distrib), "./WScrabble.ini");
		GetPrivateProfileString("Settings", "dictionnary",   "english.dic", filename, sizeof(filename), "./WScrabble.ini");
		readDictionnary(filename);
	}
	read_tops();
#ifndef OFFLINE
	WSADATA wsaData;
	if ( WSAStartup(MAKEWORD( 2, 2 ), &wsaData) != 0 ) 
	{
		cerr << "WSAStartup failed!" << endl;
		halt(1);
	}
#endif
reconnect:
	cout << "Connecting to IRC..." << endl;
	irc_init();
	{
		char servername[128], password[128], nickname[NICK_MAX], altnickname[NICK_MAX], ident[40], fullname[128];
		GetPrivateProfileString("IRC", "Server",   "irc.epiknet.org", servername, sizeof(servername), "./WScrabble.ini");
		if (strcmp(servername, "<IRC SERVER HOSTNAME>") == 0)
		{
			cerr << endl << "Configure WScrabble.ini first !!" << endl;
			halt(2);
		}
		int port = GetPrivateProfileInt("IRC", "Port", 6667, "./WScrabble.ini");
		GetPrivateProfileString("IRC", "Password",	"",				password,	sizeof(password),		"./WScrabble.ini");
		GetPrivateProfileString("IRC", "Nick",		"Scrabblor",    nickname,	sizeof(nickname),		"./WScrabble.ini");
		GetPrivateProfileString("IRC", "ANick",		"Scrabbl0r",    altnickname,sizeof(altnickname),	"./WScrabble.ini");
		GetPrivateProfileString("IRC", "Ident",		"wscrabble",	ident,		sizeof(ident),			"./WScrabble.ini");
		GetPrivateProfileString("IRC", "Fullname",	BOTFULLNAME,	fullname,	sizeof(fullname),		"./WScrabble.ini");
		GetPrivateProfileString("IRC", "Channel",	"#scrabble",	channel,	sizeof(channel),		"./WScrabble.ini");
		GetPrivateProfileString("IRC", "ChannelKey","",				channelkey,	sizeof(channelkey),		"./WScrabble.ini");
		irc_blackAndWhite = GetPrivateProfileInt("IRC", "BlackAndWhite", FALSE, "./WScrabble.ini");
		anyoneCanStop = GetPrivateProfileInt("IRC", "AnyoneCanStop", FALSE, "./WScrabble.ini");
		irc_connect(servername, port, password, nickname, altnickname, ident, "localhost", fullname);
	}
	{
		char perform[1024];
		GetPrivateProfileString("IRC", "Perform",		"",				perform,		sizeof(perform), "./WScrabble.ini");
		do_perform(perform);
		irc_flushrecv();
	}
	GetPrivateProfileString("IRC", "Owner",		"",				owner,		sizeof(owner), "./WScrabble.ini");
	irc_join(channel, channelkey);
	show_about();
	cur_state = running;
	UINT noWinner = 0;
	while (cur_state != halting)
	{
		char letters[WORD_MAX], sortedLetters[WORD_MAX];
		do
		{
			pickLetters(letters, sortedLetters);
			displayLetters(letters);
			findWords(sortedLetters);
			irc_sendmsg(channel);
			if (foundWords == 0)
				irc_sendformatline("FoundNone", "[I've found no possible words !! Let's roll again ;-)...]");
			else
				irc_sendformatline("Found", "[I've found %1!d! words, including %2!d! which contain %3!d! letters.]",foundWords,foundMaxWords,maxWordLen);
		} while (foundWords == 0);
		int clock = GetPrivateProfileInt("Delay", "max", 40, "./WScrabble.ini")*10;
#ifdef CHEAT
		displayMaxWords(sortedLetters, maxWordLen);
		cout << endl;
		clock = 1;
#endif
		int warning = clock-GetPrivateProfileInt("Delay", "warning", 30, "./WScrabble.ini")*10;
		SYSTEMTIME systemTime;
		GetSystemTime(&systemTime);
		WORD lastDayOfWeek = systemTime.wDayOfWeek;
		WORD lastHour = systemTime.wHour;
		char line[1024];
		char winningNick[NICK_MAX];
		int winningWordLen = 0;
		DWORD lastRecvTicks = GetTickCount();
		bool PINGed = false;
		do
		{
			clock--;
			if (clock == warning)
			{ // plus que 10 sec
				irc_sendmsg(channel);
				irc_sendformatline("Warning","Warning, time is nearly out!");
			}
			else if (clock == 0)
			{
				irc_sendmsg(channel);
				displayMaxWords(sortedLetters, maxWordLen);
				irc_sendformatline("Timeout", "[Time is out.] MAX words were %1", dispMaxWordsString+3);
				if (winningWordLen)
				{
					irc_sendmsg(channel);
					irc_sendformat("WinSome", "%1 gets %2!d! points! ",winningNick,winningWordLen);
					send_update_stats(winningNick,winningWordLen);
					noWinner = 0;
				}
				else if (++noWinner == GetPrivateProfileInt("Settings", "autostop", 3, "./WScrabble.ini"))
				{
					noWinner = 0;
					irc_sendmsg(channel);
					irc_sendformatline("GameOver", "[Game is over. Type !start to restart it.]");
					cur_state = stopped;
				}
				break;
			}
			Sleep(100);
			GetSystemTime(&systemTime);
			if (systemTime.wDayOfWeek != lastDayOfWeek)
			{
				if (systemTime.wDayOfWeek == 1)
				{ // we are now monday
					irc_sendmsg(channel);
					irc_sendformatline("NewWeek", "A new week is beginning ! Resetting all week scores...");
					clear_week_scores();
				}
				lastDayOfWeek = systemTime.wDayOfWeek;
			}
			if (systemTime.wHour != lastHour)
			{
				show_about();
				lastHour = systemTime.wHour;
			}
			
			if (kbhit() && (getch() == 27)) cur_state = halting;
			while (irc_recv(line))
			{
				lastRecvTicks = GetTickCount();
				PINGed = false;
				char *nickname, *ident, *hostname, *cmd, *param1, *param2, *paramtext;
				irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1, &param2, &paramtext);
				if ((strcmp(cmd, "PRIVMSG") == 0) && (stricmp(param1, channel) == 0))
				{
					irc_stripcodes(paramtext);
					while (isspace(*paramtext)) paramtext++;
					if (*paramtext == 0) continue;
					if (strnicmp(paramtext, "!r", 2) == 0)
						displayLetters(letters);
					else if (!scrabbleCmd(nickname,paramtext))
					{
						while ((*paramtext != 0) && !isalpha(*paramtext)) paramtext++;
						if (*paramtext == 0) continue;
						char *wordEnd = paramtext+1;
						while (isalpha(*wordEnd)) wordEnd++;
						*wordEnd = 0;
						if ((int) strlen(paramtext) > winningWordLen)
						{
							strupr(paramtext);
							if (checkWord(sortedLetters,paramtext))
							{
								irc_sendmsg(channel);
								strcpy(winningNick, nickname);
								winningWordLen = (int) strlen(paramtext);
								if (winningWordLen == maxWordLen)
								{
									irc_sendformat("Win", "Congratulations %1 ! There's not better [%2] !! You get %3!d! points + %4!d! bonus !",nickname,paramtext,winningWordLen,bonus);
									send_update_stats(nickname,winningWordLen+bonus);
									if (GetPrivateProfileInt("Settings", "autovoice", 1, "./WScrabble.ini"))
									{
										irc_send("MODE ");
										irc_send(channel);
										irc_send(" +v ");
										irc_sendline(nickname);
									}
									clock = 0;
								}
								else
								{
									irc_sendformatline("Word", "Not bad %1... I keep your word [%2] ! Who can say better than %3!d! letters ?",nickname,paramtext,(int) strlen(paramtext));
								}
							}
						}
					}
				}
			}
			if (!PINGed && (GetTickCount()-lastRecvTicks > 15000))
			{
				sprintf(line, "%.8X", (rand()<<16)|rand());
				irc_send("PING :");
				irc_sendline(line);
				PINGed = true;
			}
			else if (PINGed && (GetTickCount()-lastRecvTicks > 20000))
			{
				irc_end();
				goto reconnect;
			}
		} while ((cur_state == running) && clock);
		clock = GetPrivateProfileInt("Delay", "after", 10, "./WScrabble.ini")*10;
		do
		{
			Sleep(100);
			if (kbhit() && (getch() == 27)) cur_state = halting;
			while (irc_recv(line))
			{
				char *nickname, *ident, *hostname, *cmd, *param1, *param2, *paramtext;
				irc_analyze(line, &nickname, &ident, &hostname, &cmd, &param1, &param2, &paramtext);
				if ((strcmp(cmd, "PRIVMSG") == 0) && (stricmp(param1, channel) == 0))
				{
					irc_stripcodes(paramtext);
					while (isspace(*paramtext)) paramtext++;
					if (*paramtext == 0) continue;
					scrabbleCmd(nickname,paramtext);
				}
			}
		} while (((cur_state == running) && clock--) || (clock = 0, cur_state == stopped));
	}
	irc_end();
#ifndef OFFLINE
	WSACleanup();
#endif
	return 0;
}
