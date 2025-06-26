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
char shiftTransform(char letter);

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
                        if (SDL_GetModState() & SDL_KMOD_SHIFT)
                        {
                            letter = shiftTransform(letter);
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
    case SDLK_A:
        return 'a';
    case SDLK_B:
        return 'b';
    case SDLK_C:
        return 'c';
    case SDLK_D:
        return 'd';
    case SDLK_E:
        return 'e';
    case SDLK_F:
        return 'f';
    case SDLK_G:
        return 'g';
    case SDLK_H:
        return 'h';
    case SDLK_I:
        return 'i';
    case SDLK_J:
        return 'j';
    case SDLK_K:
        return 'k';
    case SDLK_L:
        return 'l';
    case SDLK_M:
        return 'm';
    case SDLK_N:
        return 'n';
    case SDLK_O:
        return 'o';
    case SDLK_P:
        return 'p';
    case SDLK_Q:
        return 'q';
    case SDLK_R:
        return 'r';
    case SDLK_S:
        return 's';
    case SDLK_T:
        return 't';
    case SDLK_U:
        return 'u';
    case SDLK_V:
        return 'v';
    case SDLK_W:
        return 'w';
    case SDLK_X:
        return 'x';
    case SDLK_Y:
        return 'y';
    case SDLK_Z:
        return 'z';
    case SDLK_1:
        return '1';
    case SDLK_2:
        return '2';
    case SDLK_3:
        return '3';
    case SDLK_4:
        return '4';
    case SDLK_5:
        return '5';
    case SDLK_6:
        return '6';
    case SDLK_7:
        return '7';
    case SDLK_8:
        return '8';
    case SDLK_9:
        return '9';
    case SDLK_0:
        return '0';
    case SDLK_APOSTROPHE:
        return '\'';
    case SDLK_COMMA:
        return ',';
    case SDLK_MINUS:
        return '-';
    case SDLK_PERIOD:
        return '.';
    case SDLK_SLASH:
        return '/';
    case SDLK_SEMICOLON:
        return ';';
    case SDLK_EQUALS:
        return '=';
    case SDLK_LEFTBRACE:
        return '[';
    case SDLK_BACKSLASH:
        return '\\';
    case SDLK_RIGHTBRACE:
        return ']';
    case SDLK_GRAVE:
        return '`';
    default:
        return '\0';
    }
}

char shiftTransform(char letter)
{
    if (letter >= 'a' && letter <= 'z')
    {
        return letter - 0x20;
    }
    else
    {
        switch (letter)
        {
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '\'':
            return '"';
        case ',':
            return '<';
        case '-':
            return '_';
        case '.':
            return '>';
        case '/':
            return '?';
        case ';':
            return ':';
        case '=':
            return '+';
        case '[':
            return '{';
        case '\\':
            return '|';
        case ']':
            return '}';
        case '`':
            return '~';
        default:
            return letter;
        }
    }
}