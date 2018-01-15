//
// Created by invik on 17/10/17.
//

#include "irc.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdarg>

int irc_socket;
char irc_buffer[8193];
size_t irc_bufLen = 0;
bool irc_blackAndWhite;
bool anyoneCanStop;
string channel;
string bot_nick;
vector<string> owner;

void init_socket() {
#ifndef OFFLINE
    irc_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (irc_socket < 0) {
        log_stderr("Cannot create a socket.");
        halt(irc_socket);
    }
#endif
}

void irc_send(const string &text) {
    log_stdout(text.c_str());
#ifndef OFFLINE
    send(irc_socket, &text[0], text.length(), 0);
#endif
}

void irc_send(char ch) {
    log_stdout(&ch);
#ifndef OFFLINE
    send(irc_socket, &ch, 1, 0);
#endif
}

void irc_send(int value) {
    char text[32];
    snprintf(text, 32, "%d", value);
    log_stdout(text);
#ifndef OFFLINE
    send(irc_socket, text, (int) strlen(text), 0);
#endif
}

void irc_sendline(const string &line) {
    log_stdout(line.c_str());
#ifndef OFFLINE
    send(irc_socket, &line[0], line.length(), 0);
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
        msleep(500);
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
    return;
#else
    struct hostent *host = gethostbyname(&servername[0]);
    if (!host) {
        log_stdout("Could not resolve server name");
        halt(2);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_addr = *(struct in_addr *) host->h_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((u_short) port);
    connect(irc_socket, (sockaddr *) &serv_addr, sizeof(serv_addr));

    msleep(500);
    irc_flushrecv();

    if (!password.empty()) {
        irc_send("PASS ");
        irc_sendline(password);
    }

    irc_sendline("NICK " + nickname);
    irc_sendline("USER " + ident + " " + localhost + " " + servername + " :" + fullname);

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
                    log_stderr("Nicknames already in use");
                    halt(6);
                }
                nickname = altnickname;
                irc_sendline("NICK " + nickname);
            }
        } else
            msleep(500);
    }

    // Time out: we were not able to connect
    log_stderr("Problem during connection");
    halt(7);
#endif
}

void do_perform(const string &perform) {
    char *token = strtok((char *)&perform[0], "|");
    while (token != nullptr) {
        token = token + strspn(token, " ");
        if (*token == '/') token++;
        char *scan = strchr(token, ' ');
        if (scan) *scan++ = '\0';
        strupr(token);
        if ((strcmp(token, "MSG") == 0) || (strcmp(token, "NOTICE") == 0)) {
            if (!scan) {
                char text[128];
                snprintf(text, 128, "Missing argument for %s on Perform= setting", token);
                log_stderr(text);
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

bool irc_want(const char *wantCmd, u_int timeout = 15000) {
#ifdef OFFLINE
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
            msleep(100);
    }
    return false;
#endif
}

void irc_join(const string &channel, const string &channelkey = nullptr) {
    irc_send("JOIN ");
    if (!channelkey.empty()) {
        irc_sendline(channel + " " + channelkey);
    } else
        irc_sendline(channel);
    irc_want("JOIN");
}

void irc_sendmsg(const string &dest) {
    irc_send("PRIVMSG " + dest + " :");
}

void irc_sendnotice(const string &dest) {
    irc_send("NOTICE " + dest + " :");
}

void irc_connect() {

    // Prepare socket & connect
    init_socket();
    irc_connect(servername, port, server_pass, bot_nick, altnickname, ident, "localhost", fullname);

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
        if (ch == 3) {  // couleur CTRL-K
            if (isdigit(*scan)) {  // suivi d'un chiffre
                if (isdigit(*++scan)) scan++; // eventuellement un 2eme chiffre
                if ((*scan == ',') && isdigit(*(scan + 1))) {  // eventellement une virgule suivie d'un chiffre
                    scan += 2;
                    if (isdigit(*scan)) scan++; // eventuellement un 2eme chiffre
                }
            }
        } else if (is_valid_char(ch))
            *text++ = ch;
    }
    *text++ = 0;  // Set null terminator
}

void irc_disconnect() {
    irc_sendline("QUIT :Game Over");
#ifndef OFFLINE
    do {
        msleep(500);
    } while (!irc_want("ERROR"));
    close(irc_socket);
#endif
}

void irc_sendformat(bool set_endl, const string & lpKeyName, const string & lpDefault, ...) {
    string buffer = cfg<string_ini_t>("Strings", lpKeyName, lpDefault);
    if (irc_blackAndWhite) irc_stripcodes(&buffer[0]);
    va_list arguments;
    char text[8192];
    va_start(arguments, lpDefault);
    vsnprintf(text, 8192, &buffer[0], arguments);
    va_end(arguments);
    log_stdout(text);
    if (set_endl) log_stdout("");
#ifndef OFFLINE
    send(irc_socket, text, (int) strlen(text), 0);
    if (set_endl)
        send(irc_socket, "\n", 1, 0);
#endif
}

void irc_close() {
    close(irc_socket);
}