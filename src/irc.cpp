//
// Created by invik on 17/10/17.
//
#include <cstdarg>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "inicpp/inicpp.h"
#include "irc.hpp"
#include "lxScrabble.hpp"
#include "mimics.hpp"

std::string servername;
int port;
std::string server_pass;
std::string altnickname;
std::string ident;
std::string fullname;
std::string channelkey;
std::string perform;

int irc_socket;
char irc_buffer[BUFFER_SIZE];
size_t irc_bufLen = 0;
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
        throw std::runtime_error("Cannot create a socket.");
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

bool irc_handlestd(line_buffer_t line)
{
    if (strncmp(line, "PING :", 6) == 0) {
        irc_send("PONG :");
        irc_sendline(line + 6);
        return true;
    }
    return false;
}

bool irc_recv(line_buffer_t line)
{
    size_t value = 0;
    ioctl(irc_socket, FIONREAD, &value);
    if (value) {
        irc_bufLen +=
            recv(irc_socket, &irc_buffer[irc_bufLen],
                 std::min<unsigned long>(value, BUFFER_SIZE - irc_bufLen), 0);
        irc_buffer[irc_bufLen] = 0;
        irc_bufLen = strlen(irc_buffer);
    }
    while (true) {
        char* scan = strchr(irc_buffer, '\n');
        if (scan) {
            size_t lineLen = scan - irc_buffer;
            if (lineLen > LINE_BUFFER_SIZE) {
                lineLen = LINE_BUFFER_SIZE;
            }
            strncpy(line, irc_buffer, lineLen - 1);
            line[lineLen - 1] = 0;
            irc_bufLen -= lineLen + 1;
            memmove(irc_buffer, &irc_buffer[lineLen + 1], irc_bufLen + 1);
            if (!irc_handlestd(line)) {
                return true;
            }
        } else {
            return false;
        }
    }
}

void irc_flushrecv()
{
    line_buffer_t line = {0};
    do {
        log(line);
    } while (irc_recv(line));
}

void irc_analyze(char* line, char** nickname, char** ident, char** hostname,
                 char** cmd, char** param1, char** param2, char** paramtext)
{
    char* scan = nullptr;
    *nickname = nullptr;
    *ident = nullptr;
    *hostname = nullptr;
    if (line[0] == ':') {
        scan = strchr(line, ' ');
        *scan = 0;
        *cmd = &scan[1];
        *nickname = &line[1];
        scan = strchr(&line[1], '!');
        if (scan) {
            *scan++ = 0;
            *ident = scan;
            scan = strchr(scan, '@');
            *scan++ = 0;
            *hostname = scan;
        }
    } else {
        *cmd = line;
    }
    scan = strchr(*cmd, ' ');
    *scan++ = 0;
    *param1 = *param2 = nullptr;
    *paramtext = strchr(scan, '\0');
    while (*scan != ':') {
        if (!*param1) {
            *param1 = scan;
        } else if (!*param2) {
            *param2 = scan;
        } else {
            --scan;
            break;
        }
        scan = strchr(scan, ' ');
        if (scan == nullptr) {
            return;
        }
        *scan++ = 0;
    }
    *paramtext = &scan[1];
}

void irc_connect(const std::string& servername, int port,
                 const std::string& password, std::string& nickname,
                 const std::string& altnickname, const std::string& ident,
                 const std::string& localhost, const std::string& fullname)
{

    struct hostent* host = nullptr;
    struct sockaddr_in serv_addr;

    if ((host = gethostbyname(servername.c_str())) == nullptr) {
        throw std::runtime_error("Could not resolve server name");
    }

    serv_addr.sin_addr = *(struct in_addr*) host->h_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((u_short) port);
    if (connect(irc_socket, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Connection failed.");
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
    line_buffer_t line;
    clock_t ticks = clock();
    while (clock() - ticks < TIMEOUT) {
        if (irc_recv(line)) {
            char *cmd = nullptr, *dummy = nullptr;

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
                    throw std::runtime_error("Nicknames already in use");
                }
                nickname = altnickname;
                irc_sendline("NICK " + nickname);
            }
        } else
            msleep(500);
    }

    // Time out: we were not able to connect
    throw std::runtime_error("Problem during connection");
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
            *(++scan) = '\0';
        }
        non_ascii_strupr(token);
        if (strcmp(token, "MSG") == 0 || strcmp(token, "NOTICE") == 0) {
            if (!scan) {
                char text[128];
                snprintf(text, 128,
                         "Missing argument for %s on Perform= setting", token);
                throw std::runtime_error(text);
            }
            if (strcmp(token, "MSG") == 0)
                irc_send("PRIVMSG");
            else
                irc_send(token);
            token = scan;
            scan = strchr(token, ' ');
            *(--token) = ' ';
            *(++scan) = '\0';
            irc_send(token);
            irc_send(" :");
            irc_sendline(scan);
        } else {
            if (scan)
                *(--scan) = ' ';
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
    line_buffer_t line;
    clock_t ticks = clock();
    while (clock() - ticks < timeout) {
        if (irc_recv(line)) {
            char *cmd = nullptr, *dummy = nullptr;
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
    std::string ret;
    ret.reserve(text.length());
    size_t expected_digit = 0;
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
        ret.push_back(ch);
    }
    return ret;
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
    char text[BUFFER_SIZE] = {0};
    va_start(arguments, &lpDefault);
    vsnprintf(text, BUFFER_SIZE, buffer.c_str(), arguments);
    va_end(arguments);
    send(irc_socket, text, strlen(text), 0);
    if (set_endl) {
        send(irc_socket, "\n", 1, 0);
    }
}

Pinger::Pinger(RandGenerator& generator)
    : d_generator{generator}, d_distrib(0, 1 << 8)
{
}

void Pinger::recv()
{
    d_lastRecv = clock();
    d_pinged = false;
}

bool Pinger::is_alive()
{
    const auto delta = clock() - d_lastRecv;
    if (d_pinged && delta > TIMEOUT) {
        return false;
    } else if (!d_pinged && delta > PING_INTERVAL) {
        auto ping = fmt::format("PING: {:04X}{:04X}\n", d_distrib(d_generator),
                                d_distrib(d_generator));
        irc_sendline(ping);
        d_pinged = true;
    }

    return true;
}