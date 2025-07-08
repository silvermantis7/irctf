#include "network.hpp"
#include <asio.hpp>
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
    connected = false;

    for (const std::pair<std::string, Channel*> c : joined)
    {
        delete c.second;
    }

    for (User* u : users)
    {
        delete u;
    }
}

static std::string readOverflow = "";
void Server::queueResponses()
{
    while (connected)
    {
        std::array<char, READ_BUF_SIZE> buf;

        size_t readLen;

        try
        {
            readLen = {socket.read_some(asio::buffer(buf))};
        }
        catch (asio::system_error& e)
        {
            if (e.code() == asio::error::eof)
            {
                return;
            }
        }

        std::string bufStr = std::string(buf.data(), readLen);

        if (bufStr.back() != '\n')
        {
            readOverflow += bufStr;
            continue;
        }
        else
        {
            bufStr = readOverflow + bufStr;
            readOverflow.clear();
        }

        queueMutex.lock();

        size_t pos;
        for (int iter = 0; (pos = bufStr.find("\r\n")) != std::string::npos;
            iter++)
        {
            std::string word = bufStr.substr(0, pos);
            std::cout << ">>> " << word << '\n';

            responseQueue.emplace_back(response::readResponse(std::move(word)));

            bufStr = bufStr.substr(pos += 2);
        }

        queueMutex.unlock();
    }

    std::cout << "!connected\n";
}

void Server::connect()
{
    if (connected)
    {
        return;
    }

    asio::connect(socket, endpoints);
    connected = true;
    queueResponsesThread = std::thread(&Server::queueResponses, this);
    queueResponsesThread.detach();
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
    connected = false;
}

void Server::quit(std::string message)
{
    send("QUIT :" + message);
    connected = false;
}

void Server::send(std::string message)
{
    asio::write(socket, asio::buffer(message + "\r\n"));
}

std::vector<response::responseVarient> Server::fetch()
{
    queueMutex.lock();
    std::vector<response::responseVarient> result(responseQueue);
    responseQueue.clear();
    queueMutex.unlock();

    return result;
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
