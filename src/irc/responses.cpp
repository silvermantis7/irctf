#include "network.hpp"
#include <exception>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace irc::response;

ParseError::ParseError(std::string error, std::vector<std::string>& words)
    : error(std::move(error))
    , words(words)
{
    if (this->words.empty())
    {
        this->error += " : {EMPTY}";
        return;
    }

    std::string wordsStr{std::accumulate(std::next(this->words.begin()),
        this->words.end(), this->words.at(0), [](std::string acc, std::string
        next) { return acc + ", " + next; })};
    this->error += " : { " + std::move(wordsStr) + " }";
}

const char* ParseError::what() const noexcept
{
    return error.c_str();
}

const std::unordered_map<std::string, Response::ResponseType> commandNames = {
    {"JOIN", Response::ResponseType::JOIN},
    {"PRIVMSG", Response::ResponseType::PRIVMSG}
};

responseVarient irc::response::readResponse(std::string raw)
{
    std::vector<std::string> words;
    std::string line; size_t pos;
    for (int iter = 0; (pos = raw.find(' ')) != std::string::npos; iter++)
    {
        words.push_back(raw.substr(0, pos));
        raw = raw.substr(++pos);
    }
    if (raw != "")
    {
        words.push_back(raw);
    }

    try
    {
        int numericID = std::stoi(words.at(2));

        if (numericID > 999)
        {
            throw std::exception();
        }
        else
        {
            return Numeric(words, numericID);
        }
    }
    catch (std::exception& e)
    {
        try
        {
            if (words.at(0) == "PING")
            {
                return Ping(std::move(words));
            }

            switch (commandNames.at(words.at(1)))
            {
            case Response::ResponseType::JOIN:
                return Join(std::move(words));
            case Response::ResponseType::PRIVMSG:
                return Privmsg(std::move(words));
            default:
                throw ParseError("Unable to determine command type", words);
                break;
            }
        }
        catch (std::out_of_range& e)
        {
            return Response(std::move(words));
        }
    }
}

Response::Response(std::vector<std::string> words) : words(words) { }

Numeric::Numeric(std::vector<std::string> words, int numericID)
    : Response(std::move(words))
    , numericID(numericID) { }

Join::Join(std::vector<std::string> words) : Response(std::move(words))
{
    try
    {
        channel = this->words.at(2);
        nick = this->words.at(0).substr(1);
        nick = nick.substr(0, nick.find('!'));
    }
    catch (std::exception& e)
    {
        throw ParseError("malformed JOIN response received", this->words);
    }
}

Ping::Ping(std::vector<std::string> words) : Response(std::move(words))
{
    try
    {
        code = this->words.at(1);
    }
    catch (std::out_of_range& e)
    {
        throw ParseError("PING response does not contain key", this->words);
    }
}

Privmsg::Privmsg(std::vector<std::string> words) : Response(std::move(words))
{
    try
    {
        nick = this->words.at(0).substr(1);
        nick = nick.substr(0, nick.find('!'));
        channel = this->words.at(2);

        message = std::accumulate(this->words.begin() + 4, this->words.end(),
            this->words.at(3).substr(1), [](std::string acc, std::string next)
            { return acc + ' ' + next; });
    }
    catch (std::out_of_range& e)
    {
        throw ParseError("malformed PRIVMSG recieved", this->words);
    }
}

void Ping::pong(Server& server)
{
    server.send("PONG " + code);
}
