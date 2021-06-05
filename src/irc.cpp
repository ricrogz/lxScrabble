//
// Created by invik on 17/10/17.
//
#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <limits>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "fmt/format.h"
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
std::array<char, BUFFER_SIZE> irc_buffer;
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

bool irc_handlestd(std::string& line)
{
    if (line.compare(0, 6, "PING :") == 0) {
        log(line);
        irc_send("PONG :");
        line.erase(0, 6);
        irc_sendline(line);
        return true;
    }
    return false;
}

bool irc_recv(std::string& line)
{
    size_t value = 0;
    ioctl(irc_socket, FIONREAD, &value);
    if (value) {
        auto ret = recv(irc_socket, irc_buffer.begin() + irc_bufLen,
                        std::min<size_t>(value, BUFFER_SIZE - irc_bufLen), 0);
        if (ret < 0) {
            throw std::runtime_error("Error retrieving data from socket.");
        }
        irc_bufLen += ret;
    }
    while (true) {
        auto scan = memchr(irc_buffer.begin(), '\n', irc_bufLen);
        if (scan != nullptr) {
            size_t lineLen =
                std::distance(irc_buffer.begin(), static_cast<char*>(scan));
            if (lineLen > LINE_BUFFER_SIZE) {
                lineLen = LINE_BUFFER_SIZE;
            }
            // skip the new line char
            line.assign(irc_buffer.begin(),
                        lineLen - (irc_buffer.at(lineLen - 1) == '\r'));
            ++lineLen;
            irc_bufLen -= lineLen;
            memmove(irc_buffer.begin(), irc_buffer.begin() + lineLen,
                    irc_bufLen);

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
    std::string line;
    while (irc_recv(line)) {
        log(line);
    }
}

void irc_analyze(std::string&& line, std::string& nickname, std::string& cmd,
                 std::string& target, std::string& text)
{
    size_t pos = 0;
    if (line[0] == ':') {
        pos = line.find(' ');
        auto scan = line.rfind('!', pos);
        if (scan != std::string::npos) {
            // we don't care about ident and hostname
            nickname.assign(line, 1, scan - 1);
        } else {
            nickname.assign(line, 1, pos - 1);
        }
        ++pos;
    } else {
        nickname.clear();
    }

    auto scan = line.find(' ', pos);
    cmd.assign(line, pos, scan - pos);
    ++scan;

    line.erase(0, scan);
    text = std::move(line);
    scan = 0;

    target.clear();
    do {
        pos = scan;
        scan = text.find(' ', pos);
        if (scan == std::string::npos) {
            break;
        }
        if (target.empty()) {
            target.assign(text, pos, scan - pos);
        }
        ++scan;
        // There may be other parameters (e.g. modes),
        // but we don't care about them.
    } while (text[scan] != ':');
    text.erase(0, scan + 1);
}

void irc_connect(const std::string& servername, int port,
                 const std::string& password, std::string& nickname,
                 const std::string& altnickname, const std::string& ident,
                 const std::string& localhost, const std::string& fullname)
{

    const auto host = gethostbyname(servername.c_str());
    if (host == nullptr) {
        throw std::runtime_error("Could not resolve server name");
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, *host->h_addr_list, host->h_length);

    auto server = reinterpret_cast<sockaddr*>(&serv_addr);
    if (connect(irc_socket, server, sizeof(serv_addr)) < 0) {
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
    std::string line;
    clock_t ticks = clock();
    while (clock() - ticks < TIMEOUT) {
        if (irc_recv(line)) {

            log(line);

            std::string cmd;
            std::string dummy;

            // Get response text
            irc_analyze(std::move(line), dummy, cmd, dummy, dummy);

            // We got a numeric indicating success
            if (cmd == "001") {
                irc_flushrecv();
                return;

                // Numerics indicating the nick is busy; use alternate
            } else if (cmd == "432" || cmd == "433") {
                if (nickname == altnickname) {
                    throw std::runtime_error("Nicknames already in use");
                }
                nickname = altnickname;
                irc_sendline("NICK " + nickname);
            }
        } else {
            msleep(500);
        }
    }

    // Time out: we were not able to connect
    throw std::runtime_error("Problem during connection");
}

void do_perform(std::string perform)
{
    char* token = strtok(const_cast<char*>(perform.data()), "|");
    while (token != nullptr) {
        token = token + strspn(token, " ");
        if (*token == '/') {
            ++token;
        }
        char* scan = strchr(token, ' ');
        if (scan) {
            *scan = '\0';
            ++scan;
        }
        non_ascii_strupr(token);
        if (strcmp(token, "MSG") == 0 || strcmp(token, "NOTICE") == 0) {
            if (!scan) {
                auto text = fmt::format(
                    "Missing argument for {} on Perform= setting", token);
                throw std::runtime_error(text);
            }
            if (strcmp(token, "MSG") == 0) {
                irc_send("PRIVMSG");
            } else {
                irc_send(token);
            }
            token = scan;
            scan = strchr(token, ' ');
            --token;
            *token = ' ';
            *scan = '\0';
            ++scan;
            irc_send(token);
            irc_send(" :");
            irc_sendline(scan);
        } else {
            if (scan) {
                --scan;
                *scan = ' ';
            }
            irc_sendline(token);
        }
        msleep(300);
        irc_flushrecv();
        token = strtok(nullptr, "|");
    }
    msleep(300);
    irc_flushrecv();
}

bool irc_want(const std::string& wantCmd, int timeout = 15000)
{
    std::string line;
    auto ticks = clock();
    while (clock() - ticks < timeout) {
        if (irc_recv(line)) {
            std::string cmd;
            std::string dummy;
            irc_analyze(std::move(line), dummy, cmd, dummy, dummy);
            if (cmd == wantCmd) {
                return true;
            }
        } else {
            msleep(100);
        }
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
        if (ret.empty() && isspace(ch)) {
            continue;
        }
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

Pinger::Pinger(RandGenerator& generator)
    : d_generator{generator},
      d_distrib(0u, std::numeric_limits<uint32_t>::max()), d_lastRecv{clock()}
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
        auto ping = fmt::format("PING :{:08X}", d_distrib(d_generator));
        irc_sendline(ping);
        d_pinged = true;
    }

    return true;
}

std::string get_fmt_template(const std::string& lpKeyName,
                             const std::string& lpDefault)
{
    auto fmt_template =
        cfg<inicpp::string_ini_t>("Strings", lpKeyName, lpDefault);
    if (irc_blackAndWhite) {
        return irc_stripcodes(fmt_template);
    }
    return fmt_template;
}