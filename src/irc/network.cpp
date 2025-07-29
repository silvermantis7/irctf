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

void Server::nick(std::string_view value)
{
    send(std::string("NICK ").append(value));
}

void Server::auth(std::string_view username, std::string_view realname)
{
    send(std::string("USER ").append(username).append(" 0 * :")
        .append(realname));
}

void Server::join(std::string_view channel)
{
    send(std::string("JOIN ").append(channel));
}

void Server::privmsg(std::string_view channel, std::string_view message)
{
    send(std::string("PRIVMSG ").append(channel).append(" :").append(message));
}

void Server::quit()
{
    send("QUIT");
    connected = false;
}

void Server::quit(std::string_view message)
{
    send(std::string("QUIT :").append(message));
    connected = false;
}

void Server::send(std::string_view message)
{
    asio::write(socket, asio::buffer(std::string(message).append("\r\n")));
}

void Server::part(std::string_view channel)
{
    send(std::string("PART ").append(channel));
}

void Server::part(std::string_view channel, std::string_view message)
{
    send(std::string("PART ").append(channel).append(" :").append(message));
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
