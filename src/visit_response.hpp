#pragma once

#include <iostream>
#include "gui/gui/log_item.hpp"
#include "irc/network.hpp"
#include "gui/gui.hpp"

template<int T> void visitResponse(
    irc::response::responseVarient& varient,
    irc::Server&
    server,
    gui::TabBar& tabBar
);

// Response
template<> void visitResponse<0>(
    irc::response::responseVarient& varient,
    irc::Server&
    server,
    gui::TabBar& tabBar
) {
    std::cout << "[+] RESPONSE\n";
}

// Numeric
template<> void visitResponse<1>(
    irc::response::responseVarient& varient,
    irc::Server&
    server,
    gui::TabBar& tabBar
) {
    std::cout << "[+] NUMERIC\n";
}

// Join
template<> void visitResponse<2>(
    irc::response::responseVarient& varient,
    irc::Server&
    server,
    gui::TabBar& tabBar
) {
    using namespace irc::response;
    Join& join = std::get<Join>(varient);

    std::cout << "[+] JOIN <" << join.channel << "> (" << join.nick << ")\n";

    if (join.nick == irc::userNick)
    {
        tabBar.addChannel(join.channel);
    }

    // check if channel 
    auto messageDisplay {
        tabBar.messageDisplays.find(join.channel)
    };

    if (messageDisplay == tabBar.messageDisplays.end())
    {
        return;
    }

    messageDisplay->second.second.logMessage(gui::log_item::Join { join.nick });
}

// Ping
template<> void visitResponse<3>(
    irc::response::responseVarient& varient,
    irc::Server& server,
    gui::TabBar& tabBar
) {
    using namespace irc::response;

    std::cout << "[+] PING\n";
    std::get<Ping>(varient).pong(server);
}

// Privmsg
template<>
void visitResponse<4>(
    irc::response::responseVarient& varient,
    irc::Server&
    server,
    gui::TabBar& tabBar
) {
    using namespace irc::response;

    std::cout << "[+] PRIVMSG\n";
    std::cout << "[" << std::get<Privmsg>(varient).channel << "] <" <<
        std::get<Privmsg>(varient).nick << "> " <<
        std::get<Privmsg>(varient).message << '\n';

    auto messageDisplay{tabBar.messageDisplays.find(std::get<Privmsg>
        (varient).channel)};

    if (messageDisplay == tabBar.messageDisplays.end())
    {
        return;
    }

    messageDisplay->second.second.logMessage(
        gui::log_item::Message {
            std::time(nullptr),
            std::get<Privmsg>(varient).nick,
            std::get<Privmsg>(varient).message
        }
    );
}

// Part
template<> void visitResponse<5>(
    irc::response::responseVarient& varient,
    irc::Server& server,
    gui::TabBar& tabBar
) {
    irc::response::Part& part = std::get<irc::response::Part>(varient);

    std::cout << "[+] PART " << part.channel << '\n';

    if (part.nick == irc::userNick)
    {
        tabBar.closeTab(std::get<irc::response::Part>(varient).channel);
    }
    else
    {
        auto messageDisplay { tabBar.messageDisplays.find(part.channel) };

        if (messageDisplay == tabBar.messageDisplays.end())
        {
            return;
        }

        messageDisplay->second.second.logMessage(
            gui::log_item::Part { part.nick, part.message }
        );
    }
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
