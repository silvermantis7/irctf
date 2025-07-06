#include <iostream>
#include <memory>
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
        std::exit(-1);
    }

    runWindow(*window);
    gui::terminate();

    std::unique_ptr<irc::Server> server;

    try
    {
        server = std::make_unique<irc::Server>("localhost", "6667");
        ResponseVisitor rVisit(*server);

        server->connect();
        server->nick("silvermantis");
        server->auth("silvermantis", "James");
        server->join("#test");

        for (;;)
        {
            std::cout << "[+] reading responses...\n";

            for (irc::response::responseVarient response : server->fetch())
            {
                std::visit(rVisit, response);
            }
        }

        server->quit();
    }
    catch (std::exception& e)
    {
        std::cerr << "[!] IRC network error: " << e.what() << '\n';
        return -1;
    }

    return 0;
}

void runWindow(gui::Window& window)
{
    using namespace gui;

    std::unique_ptr<TextBox> textBox{std::make_unique<TextBox>(window, 20, 570,
        650, 20)};
    std::unique_ptr<TabBar> tabBar{std::make_unique<TabBar>(window, 20, 15, 760,
        25)};

    std::function<void()> printInput = [&]
    {
        if (tabBar->activeTab && !textBox->textBuffer.empty())
        {
            tabBar->activeTab->second.logMessage({std::time(nullptr), "nick",
                textBox->textBuffer});
            textBox->textBuffer.clear();
        }
    };

    std::unique_ptr<Button> printButton{std::make_unique<Button>(window, 680,
        570, 100, 20, "send", std::move(printInput))};
    printInput = nullptr;

    for (;;)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        Selectable::findFocus(mouseX, mouseY);
        Selectable* inFocus = Selectable::hovered;

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
            case SDL_EVENT_KEY_DOWN:
                if (Selectable::selected && Selectable::selected->selectType
                    == Selectable::SelectType::TEXT_BOX)
                {
                    char letter = gui::readChar(event, SDL_GetModState()
                        & SDL_KMOD_SHIFT);
                    
                    if (letter)
                    {
                        static_cast<TextBox*>
                            (Selectable::selected)->writeChar(letter);
                    }
                    else if (event.key.key == SDLK_BACKSPACE)
                    {
                        if (!static_cast<TextBox*>
                            (Selectable::selected)->textBuffer.empty())
                        {
                            static_cast<TextBox*>
                                (Selectable::selected)->eraseChar();
                        }
                    }
                    else if (event.key.key == SDLK_RETURN)
                    {
                        printButton->activate();
                    }
                }

                if (event.key.key == SDLK_PAGEUP && tabBar->activeTab)
                {
                    tabBar->activeTab->second.scroll(-100);
                }
                else if (event.key.key == SDLK_PAGEDOWN && tabBar->activeTab)
                {
                    tabBar->activeTab->second.scroll(100);
                }
            }
        }

        window.clear();

        textBox->draw();
        printButton->draw();
        tabBar->draw();

        window.display();
    }
}
