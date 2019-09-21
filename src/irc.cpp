//
// Created by invik on 17/10/17.
//

#include "irc.hpp"
#include "inicpp/inicpp.h"
#include "lxScrabble.hpp"
#include "mimics.hpp"
#include <cstdarg>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/socket.h>

std::string servername;
int port;
std::string server_pass;
std::string altnickname;
std::string ident;
std::string fullname;
std::string channelkey;
std::string perform;

int irc_socket;
char irc_buffer[8193];
std::size_t irc_bufLen = 0;
bool irc_blackAndWhite;
bool anyoneCanStop;
std::string channel;
std::string bot_nick;
std::string public_nick;
std::vector<std::string> owner;

void init_socket()
{
    irc_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (irc_socket < 0) {
        log_stderr("Cannot create a socket.");
        halt(irc_socket);
    }
}

void irc_send(const std::string& text)
{
    send(irc_socket, text.c_str(), text.length(), 0);
}

void irc_sendline(const std::string& line)
{
    send(irc_socket, line.c_str(), line.length(), 0);
    send(irc_socket, "\n", 1, 0);
}

bool irc_handlestd(char line[1024])
{
    if (strncmp(line, "PING :", 6) == 0) {
        irc_send("PONG :");
        irc_sendline(line + 6);
        return true;
    }
    return false;
}

bool irc_recv(char line[1024])
{
    unsigned long value = 0;
    ioctl(irc_socket, FIONREAD, &value);
    if (value) {
        irc_bufLen += recv(
            irc_socket, irc_buffer + irc_bufLen,
            std::min<unsigned long>(value, sizeof(irc_buffer) - irc_bufLen), 0);
        irc_buffer[irc_bufLen] = 0;
        irc_bufLen = strlen(irc_buffer);
    }
    for (;;) {
        char* scan = strchr(irc_buffer, '\n');
        if (scan) {
            std::size_t lineLen = scan - irc_buffer;
            if (lineLen > 1024)
                lineLen = 1024;
            strncpy(line, irc_buffer, lineLen - 1);
            line[lineLen - 1] = 0;
            irc_bufLen -= lineLen + 1;
            memmove(irc_buffer, irc_buffer + lineLen + 1, irc_bufLen + 1);
            if (!irc_handlestd(line))
                return true;
        } else
            return false;
    }
}

void irc_flushrecv()
{
    char line[1024] = {0};
    do {
        log_stdout(line);
    } while (irc_recv(line));
}

void irc_analyze(char* line, char** nickname, char** ident, char** hostname,
                 char** cmd, char** param1, char** param2, char** paramtext)
{
    char* scan;
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
        if (!*param1)
            *param1 = scan;
        else if (!*param2)
            *param2 = scan;
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

void irc_connect(const std::string& servername, int port,
                 const std::string& password, std::string& nickname,
                 const std::string& altnickname, const std::string& ident,
                 const std::string& localhost, const std::string& fullname)
{

    struct hostent* host;
    struct sockaddr_in serv_addr;

    if ((host = gethostbyname(servername.c_str())) == nullptr) {
        log_stderr("Could not resolve server name");
        halt(2);
    }

    serv_addr.sin_addr = *(struct in_addr*) host->h_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((u_short) port);
    if (connect(irc_socket, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        log_stderr("Connection failed.");
        halt(3);
    }

    msleep(500);
    irc_flushrecv();

    if (!password.empty()) {
        irc_sendline("PASS " + password);
    }

    irc_sendline("NICK " + nickname);
    irc_sendline("USER " + ident + " " + localhost + " " + servername + " :" +
                 fullname);

    // All connection data has been send. Now, analyze answers for 45 secs
    char line[1024];
    clock_t ticks = clock();
    while (clock() - ticks < TIMEOUT) {
        if (irc_recv(line)) {
            char *cmd, *dummy;

            // Get response text
            irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy,
                        &dummy);

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
}

void do_perform(const std::string& perform)
{
    char* token = strtok((char*) perform.c_str(), "|");
    while (token != nullptr) {
        token = token + strspn(token, " ");
        if (*token == '/') {
            ++token;
        }
        char* scan = strchr(token, ' ');
        if (scan) {
            *scan++ = '\0';
        }
        non_ascii_strupr(token);
        if ((strcmp(token, "MSG") == 0) || (strcmp(token, "NOTICE") == 0)) {
            if (!scan) {
                char text[128];
                snprintf(text, 128,
                         "Missing argument for %s on Perform= setting", token);
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
            if (scan)
                *--scan = ' ';
            irc_sendline(token);
        }
        msleep(300);
        irc_flushrecv();
        token = strtok(nullptr, "|");
    }
    msleep(300);
    irc_flushrecv();
}

bool irc_want(const char* wantCmd, int timeout = 15000)
{
    char line[1024];
    clock_t ticks = clock();
    while (clock() - ticks < timeout) {
        if (irc_recv(line)) {
            char *cmd, *dummy;
            irc_analyze(line, &dummy, &dummy, &dummy, &cmd, &dummy, &dummy,
                        &dummy);
            if (strcmp(cmd, wantCmd) == 0)
                return true;
        } else
            msleep(100);
    }
    return false;
}

void irc_join(const std::string& channel, const std::string& channelkey = "")
{
    irc_send("JOIN ");
    if (!channelkey.empty()) {
        irc_sendline(channel + " " + channelkey);
    } else {
        irc_sendline(channel);
    }
    irc_want("JOIN");
}

void irc_sendmsg(const std::string& dest)
{
    irc_send("PRIVMSG " + dest + " :");
}

void irc_sendnotice(const std::string& dest)
{
    irc_send("NOTICE " + dest + " :");
}

void irc_connect()
{

    // Prepare socket & connect
    init_socket();
    irc_connect(servername, port, server_pass, bot_nick, altnickname, ident,
                "localhost", fullname);

    // Perform actions (identify registered nick, etc)
    do_perform(perform);

    // Join channel
    irc_join(channel, channelkey);
}

std::string irc_stripcodes(const std::string& text)
{
    std::ostringstream ss;
    std::size_t expected_digit = 0;
    for (const auto& ch : text) {
        if (ch == 2) { // Bold
            expected_digit = 0;
            continue;
        } else if (ch == 3) { // Color marker
            expected_digit = 1;
            continue;
        } else if (expected_digit > 0) {

            // triggers at 3 & 6
            if (expected_digit % 3 == 0 && ch != ',') {
                expected_digit = 0;
            }

            // triggers at 2 & 3
            else if (expected_digit / 2 == 1 && ch == ',') {
                expected_digit = 4;
                continue;
            } else if (isdigit(ch)) {
                ++expected_digit;
                continue;
            } else {
                expected_digit = 0;
            }
        }
        ss << ch;
    }
    return std::move(ss.str());
}

void irc_disconnect_msg(const std::string& msg)
{
    irc_sendline(msg);
    irc_want("ERROR", 5000);
    close(irc_socket);
}

void irc_disconnect()
{
    irc_disconnect_msg("QUIT :Game Over");
}

void irc_sendformat(bool set_endl, const std::string& lpKeyName,
                    const std::string& lpDefault, ...)
{
    std::string buffer =
        cfg<inicpp::string_ini_t>("Strings", lpKeyName, lpDefault);
    if (irc_blackAndWhite) {
        buffer = irc_stripcodes(buffer);
    }
    va_list arguments;
    char text[8192];
    va_start(arguments, &lpDefault);
    vsnprintf(text, 8192, buffer.c_str(), arguments);
    va_end(arguments);
    send(irc_socket, text, strlen(text), 0);
    if (set_endl) {
        send(irc_socket, "\n", 1, 0);
    }
}
