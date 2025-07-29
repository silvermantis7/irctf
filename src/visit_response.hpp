#pragma once

#include <iostream>
#include "irc/network.hpp"
#include "gui/gui.hpp"

template<int T>
void visitResponse(irc::response::responseVarient& varient, irc::Server&
    server, gui::TabBar& tabBar);

// Response
template<>
void visitResponse<0>(irc::response::responseVarient& varient, irc::Server&
    server, gui::TabBar& tabBar)
{
    std::cout << "[+] RESPONSE\n";
}

// Numeric
template<>
void visitResponse<1>(irc::response::responseVarient& varient, irc::Server&
    server, gui::TabBar& tabBar)
{
    std::cout << "[+] NUMERIC\n";
}

// Join
template<>
void visitResponse<2>(irc::response::responseVarient& varient, irc::Server&
    server, gui::TabBar& tabBar)
{
    using namespace irc::response;

    std::cout << "[+] JOIN <" << std::get<Join>(varient).channel << "> (" <<
        std::get<Join>(varient).nick << ")\n";
    tabBar.addChannel(std::get<Join>(varient).channel);
}

// Ping
template<>
void visitResponse<3>(irc::response::responseVarient& varient, irc::Server&
    server, gui::TabBar& tabBar)
{
    using namespace irc::response;

    std::cout << "[+] PING\n";
    std::get<Ping>(varient).pong(server);
}

// Privmsg
template<>
void visitResponse<4>(irc::response::responseVarient& varient, irc::Server&
    server, gui::TabBar& tabBar)
{
    using namespace irc::response;

    std::cout << "[+] PRIVMSG\n";
    std::cout << "[" << std::get<Privmsg>(varient).channel << "] <" <<
        std::get<Privmsg>(varient).nick << "> " <<
        std::get<Privmsg>(varient).message << '\n';

    using Message = gui::MessageDisplay::Message;

    auto messageDisplay{tabBar.messageDisplays.find(std::get<Privmsg>
        (varient).channel)};

    if (messageDisplay == tabBar.messageDisplays.end())
    {
        return;
    }

    messageDisplay->second.second.logMessage(Message{std::time(nullptr),
        std::get<Privmsg>(varient).nick,
        std::get<Privmsg>(varient).message});
}

// Part
template<>
void visitResponse<5>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar)
{
    std::cout << "[+] PART " << std::get<irc::response::Part>(varient).channel
        << '\n';
    tabBar.closeTab(std::get<irc::response::Part>(varient).channel);
}

template void visitResponse<0>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar);
template void visitResponse<1>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar);
template void visitResponse<2>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar);
template void visitResponse<3>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar);
template void visitResponse<4>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar);
template void visitResponse<5>(irc::response::responseVarient& varient,
    irc::Server& server, gui::TabBar& tabBar);
