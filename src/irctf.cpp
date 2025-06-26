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
char readChar(SDL_Event event);

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
    using namespace gui;

    std::unique_ptr<TextBox> textBox{std::make_unique<TextBox>(window, 20, 570,
        650, 20)};
    std::unique_ptr<MessageDisplay> messageDisplay{
        std::make_unique<MessageDisplay>(window, 20, 20, 760, 520)};

    std::function<void()> printInput = [&]
    {
        if (!textBox->textBuffer.empty())
        {
            messageDisplay->logMessage({1234, "nick", textBox->textBuffer});
            textBox->textBuffer.clear();
        }
    };

    std::unique_ptr<Button> printButton{std::make_unique<Button>(window, 680,
        570, 100, 20, "send", printInput)};

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
                    char letter = readChar(event);
                    
                    if (letter)
                    {
                        if (letter >= 'a' && letter <= 'z' &&
                            SDL_GetModState() & SDL_KMOD_SHIFT)
                        {
                            letter -= 0x20;
                        }

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
            }
        }

        window.clear();

        textBox->draw();
        printButton->draw();
        messageDisplay->draw();

        window.display();
    }
}

char readChar(SDL_Event event)
{
    switch (std::move(event.key.key))
    {
    case SDLK_SPACE:
        return ' ';
        break;
    case SDLK_A:
        return 'a';
        break;
    case SDLK_B:
        return 'b';
        break;
    case SDLK_C:
        return 'c';
        break;
    case SDLK_D:
        return 'd';
        break;
    case SDLK_E:
        return 'e';
        break;
    case SDLK_F:
        return 'f';
        break;
    case SDLK_G:
        return 'g';
        break;
    case SDLK_H:
        return 'h';
        break;
    case SDLK_I:
        return 'i';
        break;
    case SDLK_J:
        return 'j';
        break;
    case SDLK_K:
        return 'k';
        break;
    case SDLK_L:
        return 'l';
        break;
    case SDLK_M:
        return 'm';
        break;
    case SDLK_N:
        return 'n';
        break;
    case SDLK_O:
        return 'o';
        break;
    case SDLK_P:
        return 'p';
        break;
    case SDLK_Q:
        return 'q';
        break;
    case SDLK_R:
        return 'r';
        break;
    case SDLK_S:
        return 's';
        break;
    case SDLK_T:
        return 't';
        break;
    case SDLK_U:
        return 'u';
        break;
    case SDLK_V:
        return 'v';
        break;
    case SDLK_W:
        return 'w';
        break;
    case SDLK_X:
        return 'x';
        break;
    case SDLK_Y:
        return 'y';
        break;
    case SDLK_Z:
        return 'z';
        break;
    default:
        return '\0';
    }
}