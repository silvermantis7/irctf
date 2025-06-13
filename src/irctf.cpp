#include <array>
#include <chrono>
#include <iostream>
#include <memory>
// #include <variant>
#include "irc/network.hpp"
#include "gui/gui.hpp"
#include <math.h>

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
        std::cout << "[+] JOIN <" << response.channel << "> (" << response.nick
            << ")\n";
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

void runWindow(gui::Window& window);

int main(int argc, char* argv[])
{
    std::cout << " IRCTF v0.1 \n"
                 "############\n\n";
                 
    gui::init();
    std::unique_ptr<gui::Window> window(new gui::Window(800, 600, "IRCTF"));
    runWindow(*window);

    gui::terminate();

    // try
    // {
    //     irc::Server server = irc::Server("localhost", "6667");
    //     ResponseVisitor rVisit(server);

    //     server.connect();
    //     server.nick("silvermantis");
    //     server.auth("silvermantis", "James");
    //     server.join("#test");

    //     for (;;)
    //     {
    //         std::cout << "[+] reading responses...\n";

    //         for (irc::response::responseVarient response : server.fetch())
    //         {
    //             std::visit(rVisit, response);
    //         }
    //     }

    //     server.quit();
    // }
    // catch (std::exception& e)
    // {
    //     std::cerr << "[!] IRC network error: " << e.what() << '\n';
    //     return -1;
    // }

    return 0;
}

void runWindow(gui::Window& window)
{
    std::array<std::unique_ptr<gui::Button>, 7> buttons;

    gui::Button* lone = new gui::Button(window, 200, 200, 50, 20, "hello",
        nullptr);
    delete lone;

    for (int iter = 0; iter < buttons.size(); iter++)
    {
        buttons.at(iter) = std::make_unique<gui::Button>(window, 10,
            30 + iter * 25, 60, 20, "hello", nullptr);
    }

    buttons.at(1)->activate = []{ std::cout << "[+] button clicked\n"; };

    for (;;)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        gui::Hoverable::findFocus(mouseX, mouseY);
        gui::Hoverable* inFocus = gui::Hoverable::current;

        SDL_Event event;
        while (window.pollEvents(event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                return;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (inFocus && inFocus->hoverType
                    == gui::Hoverable::HoverTypes::BUTTON
                    && static_cast<gui::Button*>(inFocus)->activate)
                {
                    static_cast<gui::Button*>(inFocus)->activate();
                }
                break;
            }
        }

        window.clear();

        double offset = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();

        for (int iter = 0; iter < buttons.size(); iter++)
        {
            auto& button = buttons.at(iter);

            // int posXSwap = button->posX;
            // button->posX += std::sin(offset / 200.f) * iter * 30 + iter * 30;
            buttons.at(iter)->draw();
            // button->posX = posXSwap;
        }

        window.display();
    }
}