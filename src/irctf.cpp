#include <algorithm>
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

    std::unique_ptr<gui::Window> window;

    try
    {
        gui::init();
        window = std::make_unique<gui::Window>(800, 600, "IRCTF");
    }
    catch (std::exception& e)
    {
        std::cerr << "GUI error: " << e.what() << '\n';
    }

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

    for (int iter = 0; iter < buttons.size(); iter++)
    {
        buttons.at(iter) = std::make_unique<gui::Button>(window, 10,
            30 + iter * 25, 60, 20, "hello", nullptr);
    }

    buttons.at(1)->activate = []{ std::cout << "[+] button clicked\n"; };

    std::unique_ptr<gui::TextBox>
        textBox(std::make_unique<gui::TextBox>(window, 200, 200, 200, 20));

    for (;;)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        gui::Selectable::findFocus(mouseX, mouseY);
        gui::Selectable* inFocus = gui::Selectable::hovered;

        SDL_Event event;
        while (window.pollEvents(event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                return;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (inFocus)
                {
                    inFocus->select();
                }

                break;
            }
        }

        window.clear();

        double offset = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();

        for (std::unique_ptr<gui::Button>& button : buttons)
        {
            button->draw();
        }

        textBox->draw();

        window.display();
    }
}