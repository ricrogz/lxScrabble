//
// Created by invik on 17/10/17.
//

#include "irc.h"

#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int irc_socket;
char irc_buffer[8193];
size_t irc_bufLen = 0;
bool irc_blackAndWhite;
bool anyoneCanStop;
string owner;

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
    line;
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

    char line[1024];
    clock_t ticks = clock();
    while (clock() - ticks < 45000) {
        if (irc_recv(line)) {
            char *cmd, *dummy;
            irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy, &dummy);
            if (strcmp(cmd, "001") == 0) {
                irc_flushrecv();
                return;
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

    cerr << "Problem during connection" << endl;
    halt(7);
#endif
}

void get_connection() {
    INIReader reader("WScrabble.ini");

    // Check error
    int error_check = reader.ParseError();
    if (error_check < 0) {
        cout << "Can't load 'WScrabble.ini'\n";
        halt(error_check);
    }

    // Connection data
    string servername = reader.Get("IRC", "Server", DEFAULT_SERVER);
    // TODO: is this really necessary? -- remove it
    if (strcmp(servername.c_str(), "<IRC SERVER HOSTNAME>") == 0) {
        cerr << endl << "Configure WScrabble.ini first !!" << endl;
        halt(2);
    }
    int port = (int) reader.GetInteger("IRC", "Port", DEFAULT_PORT);

    // IRC parameters
    string nickname = reader.Get("IRC", "Nick", DEFAULT_NICK);
    string server_pass = reader.Get("IRC", "Password", "");
    string altnickname = reader.Get("IRC", "ANick", DEFAULT_ANICK);
    string ident = reader.Get("IRC", "Ident", DEFAULT_IDENT);
    string fullname = reader.Get("IRC", "Fullname", BOTFULLNAME);

    // Channel
    string channel = reader.Get("IRC", "Channel", DEFAULT_CHANNEL);
    string channelkey = reader.Get("IRC", "ChannelKey", DEFAULT_CHANNEL_KEY);

    // Other configs
    irc_blackAndWhite = (bool) reader.GetInteger("IRC", "BlackAndWhite", 0);
    anyoneCanStop = (bool) reader.GetInteger("IRC", "AnyoneCanStop", 0);
    string perform = reader.Get("IRC", "Perform", "");
    owner = reader.Get("IRC", "Owner", "");

    // Connect
    irc_connect(servername, port, server_pass, nickname, altnickname, ident, "localhost", fullname);
    //do_perform(perform);
    //irc_flushrecv();

}

void irc_connect() {
    cout << "Connecting to IRC..." << endl;
    init_socket();

    // Connect
    get_connection();

    /*
    irc_join(channel, channelkey);
    show_about();
    cur_state = running;
    UINT noWinner = 0;
     */
}