#include <iostream>
#include <variant>
#include "irc/network.hpp"

struct ResponseVisitor
{
    irc::Server& server;
    ResponseVisitor(irc::Server& server) : server(server) { }

    void operator() (const irc::response::Response&)
    {
        std::cout << "[+] RESPONSE\n";
    };

    void operator() (const irc::response::Join& response)
    {
        std::cout << "[+] JOIN <" << response.channel << ">\n";
        server.join(response.channel);
    };

    void operator() (const irc::response::Numeric&)
    {
        std::cout << "[+] NUMERIC\n";
    };

    void operator() (irc::response::Ping& response)
    {
        std::cout << "[+] PING\n";
        response.pong(server);
    };

    void operator() (irc::response::Privmsg& response)
    {
        std::cout << "[+] PRIVMSG\n";
        std::cout << "[" << response.channel << "] <" << response.nick << "> "
            << response.message << '\n';
    };
};

int main(int argc, char* argv[])
{
    std::cout << " IRCTF v0.1 \n"
                 "############\n\n";

    try
    {
        irc::Server server = irc::Server("localhost", "6667");
        ResponseVisitor rVisit(server);

        server.connect();
        server.nick("silvermantis");
        server.auth("silvermantis", "James");
        server.join("#test");

        for (;;)
        {
            std::cout << "[+] reading responses...\n";

            for (irc::response::responseVarient response : server.fetch())
            {
                std::visit(rVisit, response);
            }
        }

        server.quit();
    }
    catch (std::exception& e)
    {
        std::cerr << "[!] IRC network error: " << e.what() << '\n';
        return -1;
    }

    return 0;
}
