//
// Created by invik on 17/10/17.
//

#include "irc.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdarg>

int irc_socket;
char irc_buffer[8193];
size_t irc_bufLen = 0;
bool irc_blackAndWhite;
bool anyoneCanStop;
string owner;
string channel;

void init_socket() {
#ifndef OFFLINE
    irc_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (irc_socket < 0) {
        cout << "Cannot create a socket." << endl;
        halt(irc_socket);
    }
#endif
}

void irc_send(const char *text) {
    cout << text;
#ifndef OFFLINE
    send(irc_socket, text, (int) strlen(text), 0);
#endif
}

void irc_send(char ch) {
    cout << ch;
#ifndef OFFLINE
    send(irc_socket, &ch, 1, 0);
#endif
}

void irc_send(int value) {
    cout << value;
    char text[32];
    snprintf(text, 32, "%d", value);
#ifndef OFFLINE
    send(irc_socket, text, (int) strlen(text), 0);
#endif
}

void irc_sendline(const char *line) {
    cout << line << endl;
#ifndef OFFLINE
    send(irc_socket, line, (int) strlen(line), 0);
    send(irc_socket, "\n", 1, 0);
#endif
}

bool irc_handlestd(char line[1024]) {
    if (strncmp(line, "PING :", 6) == 0) {
        irc_send("PONG :");
        irc_sendline(line + 6);
        return true;
    }
    return false;
}

bool irc_recv(char line[1024]) {
#ifndef OFFLINE
    u_long value = 0;
    ioctl(irc_socket, FIONREAD, &value);
    if (value) {
        irc_bufLen += recv(irc_socket, irc_buffer + irc_bufLen, min(value, sizeof(irc_buffer) - irc_bufLen), 0);
        irc_buffer[irc_bufLen] = 0;
        irc_bufLen = strlen(irc_buffer);
    }
    for (;;) {
        char *scan = strchr(irc_buffer, '\n');
        if (scan) {
            auto lineLen = (size_t) (scan - irc_buffer);
            if (lineLen > 1024) lineLen = 1024;
            strncpy(line, irc_buffer, lineLen - 1);
            line[lineLen - 1] = 0;
            irc_bufLen -= lineLen + 1;
            memmove(irc_buffer, irc_buffer + lineLen + 1, irc_bufLen + 1);
            cout << "<- " << line << endl;
            if (!irc_handlestd(line))
                return true;
        } else
            return false;
    }
#else
    return false;
#endif
}

void irc_flushrecv() {
    char line[1024];
    do {
        while (irc_recv(line));
        usleep(500);
    } while (irc_recv(line));
}

void irc_analyze(char *line, char **nickname, char **ident, char **hostname, char **cmd, char **param1, char **param2,
                 char **paramtext) {
    char *scan;
    *nickname = *ident = *hostname = nullptr;
    if (line[0] == ':') {
        scan = strchr(line, ' ');
        *scan = 0;
        *cmd = scan + 1;
        *nickname = line + 1;
        scan = strchr(line + 1, '!');
        if (scan) {
            *scan++ = 0;
            *ident = scan;
            scan = strchr(scan, '@');
            *scan++ = 0;
            *hostname = scan;
        }
    } else
        *cmd = line;
    scan = strchr(*cmd, ' ');
    *scan++ = 0;
    *param1 = *param2 = nullptr;
    *paramtext = strchr(scan, '\0');
    while (*scan != ':') {
        if (!*param1) *param1 = scan;
        else if (!*param2) *param2 = scan;
        else {
            scan--;
            break;
        }
        scan = strchr(scan, ' ');
        if (scan == nullptr)
            return;
        *scan++ = 0;
    }
    *paramtext = scan + 1;
}

void irc_connect(const string &servername, int port, const string &password, string &nickname,
                 const string &altnickname, const string &ident, const string &localhost, const string &fullname) {
#ifdef OFFLINE
    servername, port, password, nickname, altnickname, ident, localhost, fullname;
#else
    struct hostent *host = gethostbyname(servername.c_str());
    if (!host) {
        cerr << "Could not resolve server name" << endl;
        halt(2);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_addr = *(struct in_addr *) host->h_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((u_short) port);
    connect(irc_socket, (sockaddr *) &serv_addr, sizeof(serv_addr));

    usleep(500);
    irc_flushrecv();

    if (!password.empty()) {
        irc_send("PASS ");
        irc_sendline(password.c_str());
    }

    irc_send("NICK ");
    irc_sendline(nickname.c_str());

    irc_send("USER ");
    irc_send(ident.c_str());
    irc_send(' ');
    irc_send(localhost.c_str());
    irc_send(' ');
    irc_send(servername.c_str());
    irc_send(" :");
    irc_sendline(fullname.c_str());

    // All connection data has been send. Now, analyze answers for 45 secs
    char line[1024];
    clock_t ticks = clock();
    while (clock() - ticks < CONNECT_TIMEOUT) {
        if (irc_recv(line)) {
            char *cmd, *dummy;

            // Get response text
            irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy, &dummy);

            // We got a numeric indicating success
            if (strcmp(cmd, "001") == 0) {
                irc_flushrecv();
                return;

                // Numerics indicating the nick is busy; use alternate
            } else if ((strcmp(cmd, "432") == 0) || (strcmp(cmd, "433") == 0)) {
                if (nickname == altnickname) {
                    cerr << "Nicknames already in use" << endl;
                    halt(6);
                }
                nickname = altnickname;
                irc_send("NICK ");
                irc_sendline(nickname.c_str());
            }
        } else
            usleep(500);
    }

    // Time out: we were not able to connect
    cerr << "Problem during connection" << endl;
    halt(7);
#endif
}

void do_perform(const string &perform) {
    char *token = strtok((char *) perform.c_str(), "|");
    while (token != nullptr) {
        token = token + strspn(token, " ");
        if (*token == '/') token++;
        char *scan = strchr(token, ' ');
        if (scan) *scan++ = '\0';
        strupr(token);
        if ((strcmp(token, "MSG") == 0) || (strcmp(token, "NOTICE") == 0)) {
            if (!scan) {
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
        } else {
            if (scan) *--scan = ' ';
            irc_sendline(token);
        }
        token = strtok(nullptr, "|");
    }

}

bool irc_want(const char *wantCmd, uint timeout = 15000) {
#ifdef OFFLINE
    wantCmd, timeout;
    return true;
#else
    char line[1024];
    clock_t ticks = clock();
    while (clock() - ticks < timeout) {
        if (irc_recv(line)) {
            char *cmd, *dummy;
            irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy, &dummy);
            if (strcmp(cmd, wantCmd) == 0)
                return true;
        } else
            usleep(100);
    }
    return false;
#endif
}

void irc_join(const string &channel, const string &channelkey = nullptr) {
    irc_send("JOIN ");
    if (!channelkey.empty()) {
        irc_send(channel.c_str());
        irc_send(" ");
        irc_sendline(channelkey.c_str());
    } else
        irc_sendline(channel.c_str());
    irc_want("JOIN");
}

void irc_sendmsg(const char *dest) {
    irc_send("PRIVMSG ");
    irc_send(dest);
    irc_send(" :");
}

void irc_sendnotice(const char *dest) {
    irc_send("NOTICE ");
    irc_send(dest);
    irc_send(" :");
}

void irc_connect() {

    /* Read cfg with global reader */

    // Connection data
    string servername = cfg<string>("IRC", "Server", DEFAULT_SERVER);
    // TODO: is this really necessary? -- remove it
    if (strcmp(servername.c_str(), "<IRC SERVER HOSTNAME>") == 0) {
        cerr << endl << "Configure lxScrabble.ini first !!" << endl;
        halt(2);
    }
    int port = (int) cfg<unsigned_ini_t>("IRC", "Port", DEFAULT_PORT);

    // IRC parameters
    string nickname = cfg<string>("IRC", "Nick", DEFAULT_NICK);
    string server_pass = cfg<string>("IRC", "Password", "");
    string altnickname = cfg<string>("IRC", "ANick", DEFAULT_ANICK);
    string ident = cfg<string>("IRC", "Ident", DEFAULT_IDENT);
    string fullname = cfg<string>("IRC", "Fullname", BOTFULLNAME);

    // Channel
    channel = cfg<string>("IRC", "Channel", DEFAULT_CHANNEL);
    string channelkey = cfg<string>("IRC", "ChannelKey", DEFAULT_CHANNEL_KEY);

    // Other configs
    irc_blackAndWhite = (bool) cfg<unsigned_ini_t>("IRC", "BlackAndWhite", false);
    anyoneCanStop = (bool) cfg<unsigned_ini_t>("IRC", "AnyoneCanStop", false);
    string perform = cfg<string>("IRC", "Perform", "");
    owner = cfg<string>("IRC", "Owner", "");

    // Prepare socket & connect
    init_socket();
    irc_connect(servername, port, server_pass, nickname, altnickname, ident, "localhost", fullname);

    // Perform actions (identify registered nick, etc)
    do_perform(perform);
    irc_flushrecv();

    // Join channel
    irc_join(channel, channelkey);
}

void irc_stripcodes(char *text) {
    char *scan = text;
    char ch;
    while ((ch = *scan++) != 0) {
        if (ch == 3) // couleur CTRL-K
        {
            if (isdigit(*scan)) // suivi d'un chiffre
            {
                if (isdigit(*++scan)) scan++; // eventuellement un 2eme chiffre
                if ((*scan == ',') && isdigit(*(scan + 1))) // eventellement une virgule suivie d'un chiffre
                {
                    scan += 2;
                    if (isdigit(*scan)) scan++; // eventuellement un 2eme chiffre
                }
            }
        } else if (ch >= 32)
            *text++ = ch;
    }
    *text++ = 0;
}

void irc_disconnect() {
    irc_sendline("QUIT :Game Over");
#ifndef OFFLINE
    do {
        usleep(500);
    } while (!irc_want("ERROR"));
    close(irc_socket);
#endif
}

void irc_sendformat(bool set_endl, const string &lpKeyName, const string &lpDefault, ...) {
    string sbuffer = cfg<string>("Strings", lpKeyName, lpDefault);
    auto *buffer = (char *) sbuffer.c_str();
    if (irc_blackAndWhite) irc_stripcodes(buffer);
    va_list arguments;
    char text[8192];
    va_start(arguments, lpDefault);
    vsnprintf(text, 8192, buffer, arguments);
    va_end(arguments);
    cout << text;
    if (set_endl)
        cout << endl;
#ifndef OFFLINE
    send(irc_socket, text, (int) strlen(text), 0);
    if (set_endl)
        send(irc_socket, "\n", 1, 0);
#endif
}
