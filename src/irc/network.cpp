#include "network.hpp"
#include <asio/impl/read.hpp>
#include <string>
#include <iostream>
#include <utility>

#define READ_BUF_SIZE 512

using namespace irc;

Server::Server(std::string host, std::string port)
    : host(host)
    , port(port)
    , resolver(io_context)
    , endpoints(resolver.resolve(host, port))
    , socket(io_context) { }

Server::~Server()
{
    for (const std::pair<std::string, Channel*> c : joined)
    {
        delete c.second;
    }

    for (User* u : users)
    {
        delete u;
    }
}

void Server::connect()
{
    asio::connect(socket, endpoints);
}

void Server::nick(std::string value)
{
    send("NICK " + value);
}

void Server::auth(std::string username, std::string realname)
{
    send("USER " + username + " 0 * :" + realname);
}

void Server::join(std::string channel)
{
    send("JOIN " + channel);
}

void Server::quit()
{
    send("QUIT");
}

void Server::quit(std::string message)
{
    send("QUIT :" + message);
}

void Server::send(std::string message)
{
    asio::write(socket, asio::buffer(message + "\r\n"));
}

static std::string readOverflow = "";
std::vector<response::responseVarient> Server::fetch()
{
    std::vector<response::responseVarient> responses;

    std::array<char, READ_BUF_SIZE> buf;
    size_t readLen = socket.read_some(asio::buffer(buf));
    std::string bufStr = std::string(buf.data(), readLen);

    if (bufStr.back() != '\n')
    {
        readOverflow += bufStr;
        return { };
    }
    else
    {
        bufStr = readOverflow + bufStr;
        readOverflow.clear();
    }

    size_t pos;
    for (int iter = 0; (pos = bufStr.find("\r\n")) != std::string::npos; iter++)
    {
        std::string word = bufStr.substr(0, pos);
        std::cout << ">>> " << word << '\n';

        response::responseVarient response
            = response::readResponse(std::move(word));
        responses.push_back(std::move(response));

        bufStr = bufStr.substr(pos += 2);
    }

    return responses;
}

User::User(std::string nick, std::string username)
    : nick(nick)
    , username(username) { }

User::~User() { }

void User::privmsg(std::string content)
{
    server->send("PRIVMSG " + nick + " :" + content);
}

Channel::Channel(std::string name)
    : name(name) { }

void Channel::part()
{
    server->send("PART " + name);
}

void Channel::part(std::string message)
{
    server->send("PART " + name + " :" + message);
}

void Channel::privmsg(std::string message)
{
    server->send("PRIVMSG " + name + " :" + message);
}
