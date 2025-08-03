#include <iostream>
#include <memory>
#include "gui/gui/log_item.hpp"
#include "irc/network.hpp"
#include "gui/gui.hpp"
#include <math.h>
#include "visit_response.hpp"
#include <ranges>

void runWindow(gui::Window& window, irc::Server& server);

int main(int argc, char* argv[])
{
    std::cout << " IRCTF v0.1 \n"
                 "############\n\n";

    std::unique_ptr<irc::Server> server;

    try
    {
        server = std::make_unique<irc::Server>("localhost", "6667");
        server->connect();

        server->nick("silvermantis");
        server->auth("silvermantis", "James");
        server->join("#test");
    }
    catch(const std::exception& e)
    {
        std::cerr << "[!] IRC network error: " << e.what() << '\n';
        return -1;
    }

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

    runWindow(*window, *server);
    gui::terminate();

    server->quit();

    return 0;
}

void runWindow(gui::Window& window, irc::Server& server)
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
            if (textBox->textBuffer.front() == '/')
            {
                textBox->textBuffer.erase(0, 1);

                if (!textBox->textBuffer.empty()
                    && textBox->textBuffer.front() != '/')
                {
                    std::vector<std::string_view> commandWords;
                    for (const auto& word : std::views::split(
                        textBox->textBuffer, ' '))
                    {
                        commandWords.emplace_back(word.begin(), word.end());
                    }

                    if (commandWords.front() == "join"
                        && commandWords.size() == 2)
                    {
                        server.join(commandWords.at(1));
                    }
                    else if (commandWords.front() == "part")
                    {
                        if (commandWords.size() == 2)
                        {
                            server.part(commandWords.at(1).data());
                        }
                        else if (commandWords.size() >= 3)
                        {
                            server.part(commandWords.at(1), std::string_view(
                                commandWords.at(2).begin(),
                                commandWords.back().end()));
                        }
                    }

                    textBox->textBuffer.clear();

                    return;
                }
            }

            tabBar->activeTab->second.logMessage(log_item::Message {
                std::time(nullptr), "nick",
                textBox->textBuffer
            });

            auto activeChannel{tabBar->activeTab->first->getName};

            if (activeChannel != "global")
            {
                server.privmsg(activeChannel, textBox->textBuffer);
            }

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

        // read keyboard and mouse events
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
                    if (SDL_GetModState() & SDL_KMOD_CTRL)
                    {
                        auto* target = &tabBar->messageDisplays.at("global");

                        // find tab to the left
                        for (auto msgDisplay = tabBar->messageDisplays.begin();
                            msgDisplay != tabBar->messageDisplays.end();
                            ++msgDisplay)
                        {
                            if (msgDisplay->second.first->posX
                                < tabBar->activeTab->first->posX
                                && msgDisplay->second.first->posX
                                > target->first->posX)
                            {
                                target = &msgDisplay->second;
                            }
                        }

                        tabBar->activeTab = target;
                    }
                    else
                    {
                        tabBar->activeTab->second.scroll(-100);
                    }
                }
                else if (event.key.key == SDLK_PAGEDOWN && tabBar->activeTab)
                {
                    if (SDL_GetModState() & SDL_KMOD_CTRL)
                    {
                        auto* target = tabBar->activeTab;

                        // find tab to the right
                        for (auto msgDisplay = tabBar->messageDisplays.begin();
                            msgDisplay != tabBar->messageDisplays.end();
                            ++msgDisplay)
                        {
                            if (target == tabBar->activeTab)
                            {
                                if (
                                    msgDisplay->second.first->posX
                                    > target->first->posX
                                ) {
                                    target = &msgDisplay->second;
                                }
                            }
                            else if (
                                msgDisplay->second.first->posX
                                > tabBar->activeTab->first->posX
                                && msgDisplay->second.first->posX
                                < target->first->posX
                            ) {
                                target = &msgDisplay->second;
                            }
                        }

                        tabBar->activeTab = target;
                    }
                    else
                    {
                        tabBar->activeTab->second.scroll(100);
                    }
                }
            }
        }

        try
        {
            for (irc::response::responseVarient response : server.fetch())
            {
                std::cout << "[+] received message\n";

                switch (response.index())
                {
                case 0:
                    visitResponse<0>(response, server, *tabBar);
                    break;
                case 1:
                    visitResponse<1>(response, server, *tabBar);
                    break;
                case 2:
                    visitResponse<2>(response, server, *tabBar);
                    break;
                case 3:
                    visitResponse<3>(response, server, *tabBar);
                    break;
                case 4:
                    visitResponse<4>(response, server, *tabBar);
                    break;
                case 5:
                    visitResponse<5>(response, server, *tabBar);
                    break;
                }
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "[!] IRC network error: " << e.what() << '\n';
            return;
        }

        window.clear();

        textBox->draw();
        printButton->draw();
        tabBar->draw();

        window.display();
    }
}
