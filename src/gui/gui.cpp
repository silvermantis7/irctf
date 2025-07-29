#include "gui.hpp"
#include "blend2d/geometry.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <blend2d.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>

#ifndef _WIN32
#include <fontconfig/fontconfig.h>
#endif

using namespace gui;

GuiError::GuiError(std::string message) : message(message) { }

const char* GuiError::what() const noexcept
{
    return message.c_str();
}

void gui::init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw GuiError("failed to initialize SDL");
    }

    #ifdef _WIN32
    std::string fontPath = "C:\\Windows\\Fonts\\Arial.ttf";
    #else
    FcPattern* fcPattern = FcPatternCreate();
    FcPatternAddString(fcPattern, FC_FAMILY, (const FcChar8*)"Noto Serif");
    FcPatternAddString(fcPattern, FC_STYLE, (const FcChar8*)"Regular");
    FcResult fcResult;
    FcPattern* fcMatch = FcFontMatch(0, fcPattern, &fcResult);
    FcChar8* fontfile = nullptr;
    if (!fcMatch || FcPatternGetString(fcMatch, FC_FILE, 0, &fontfile)
        != FcResultMatch || !fontfile)
    {
        throw GuiError("failed to locate font");
    }
    std::string fontPath = (char*)fontfile;
    FcPatternDestroy(fcPattern);
    FcPatternDestroy(fcMatch);
    #endif

    BLResult fontLoadResult = gui::blFontFace.createFromFile(fontPath.c_str());

    if (fontLoadResult != BL_SUCCESS)
    {
        throw GuiError("failed to load font");
    }

    blFont.createFromFace(blFontFace, 15.f);
}

void gui::terminate()
{
    SDL_Quit();
}

Window::Window(int width, int height, std::string title)
    : window(SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL))
    , pixels{std::make_unique<std::vector<uint32_t>>(width * height)}
    , width{width}
    , height{height}
{
    if (!window)
    {
        throw GuiError("failed to spawn window");
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer)
    {
        throw GuiError("failed to create renderer");
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!texture)
    {
        throw GuiError("failed to create texture");
    }

    blImage.createFromData(width, height, BL_FORMAT_PRGB32, pixels->data(),
        width * 4);
    blContext = BLContext(blImage);
}

Window::~Window()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Window::clear()
{
    blContext.begin(blImage);
    blContext.clearAll();
    SDL_SetRenderDrawColor(renderer, 0x0a, 0x0b, 0x18, 0xff);
    SDL_RenderClear(renderer);
}

void Window::display()
{
    blContext.end();
    SDL_UpdateTexture(texture, nullptr, pixels->data(),
        width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool Window::pollEvents(SDL_Event& event)
{
    return SDL_PollEvent(&event);
}

Widget::Widget(Window& window, double posX, double posY, double width,
    double height)
    : window{window}
    , posX{posX}
    , posY{posY}
    , width{width}
    , height{height} { }

std::vector<Selectable*> Selectable::existing;
Selectable* Selectable::hovered = nullptr;
Selectable* Selectable::selected = nullptr;

Selectable::Selectable(Window& window, double posX, double posY, double width,
    double height, SelectType selectType)
    : Widget(window, posX, posY, width, height)
    , selectType{selectType}
{
    existing.push_back(this);
}

Selectable::~Selectable()
{
    existing.erase(std::remove(existing.begin(), existing.end(), this));
}

void Selectable::findFocus(double mouseX, double mouseY)
{
    for (Selectable* hoverable : existing)
    {
        if (mouseX >= hoverable->posX
            && mouseX <= (hoverable->posX + hoverable->width)
            && mouseY >= hoverable->posY
            && mouseY <= (hoverable->posY + hoverable->height))
        {
            hovered = hoverable;
            return;
        }
    }

    hovered = nullptr;
}

Button::Button(Window& window, double posX, double posY, double width,
    double height, std::string label, std::function<void()>&& activate)
    : Selectable(window, posX, posY, width, height, SelectType::BUTTON)
    , label{label}
    , activate{activate} { }

void Button::draw()
{
    draw(Selectable::hovered == this);
}

void Button::select()
{
    if (activate)
    {
        activate();
    }

    selected = nullptr;
}

void Button::draw(bool highlight)
{
    BLRoundRect roundRect(posX, posY, width, height, 2);
    window.blContext.fillRoundRect(roundRect,
        highlight ? borderColor : bgColor);
    window.blContext.setStrokeWidth(1.5);
    window.blContext.strokeRoundRect(roundRect, borderColor);

    BLGlyphBuffer glyphBuffer;
    BLTextMetrics textMetrics;
    glyphBuffer.setUtf8Text(label.c_str());
    blFont.shape(glyphBuffer);
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    double textWidth = textMetrics.boundingBox.x1 - textMetrics.boundingBox.x0;
    double textHeight = blFont.metrics().ascent - blFont.metrics().descent;

    window.blContext.setFillStyle(textColor);
    window.blContext.fillUtf8Text(BLPoint(posX + width / 2.f - textWidth / 2.f,
        posY + height - (height - textHeight) / 2.f), blFont, label.c_str());
}

TextBox::TextBox(Window& window, double posX, double posY, double width,
    double height)
    : Selectable(window, posX, posY, width, height, SelectType::TEXT_BOX) { }

void TextBox::draw()
{
    draw(Selectable::hovered == this);
}

void TextBox::draw(bool highlight)
{
    BLRect rect(posX, posY, width, height);
    window.blContext.fillRect(rect, highlight ? highlightColor : bgColor);
    window.blContext.setStrokeWidth(1.f);
    window.blContext.strokeRect(rect, borderColor);

    int textStartX = posX + 3;
    int textEndX = textStartX;

    // draw text if any
    if (!textBuffer.empty())
    {
        BLGlyphBuffer glyphBuffer;
        glyphBuffer.setUtf8Text(textBuffer.c_str());
        blFont.shape(glyphBuffer);
        BLTextMetrics textMetrics;
        blFont.getTextMetrics(glyphBuffer, textMetrics);

        textEndX += textMetrics.advance.x;

        double overflowWidth = textEndX - (posX + width - 16);
        if (overflowWidth > 0)
        {
            textStartX -= overflowWidth;
            textEndX -= overflowWidth;
        }

        double textWidth = textMetrics.boundingBox.x1
            - textMetrics.boundingBox.x0;
        double textHeight = blFont.metrics().ascent - blFont.metrics().descent;
        window.blContext.setFillStyle(textColor);
        window.blContext.clipToRect(BLRect(posX, posY, width, height));
        window.blContext.fillUtf8Text(BLPoint(textStartX, posY + height -
            (height - textHeight) / 2.f), blFont, textBuffer.c_str());
        window.blContext.restoreClipping();
    }
    
    if (selected == this)
    {
        BLLine cursor = BLLine(textEndX + 3, posY + height - 3, textEndX + 13,
            posY + height - 3);
        window.blContext.strokeLine(cursor, textColor);
    }
}

void TextBox::select()
{
    selected = this;
}

void TextBox::writeChar(char input)
{
    textBuffer += input;
}

void TextBox::eraseChar()
{
    textBuffer.pop_back();
}

MessageDisplay::MessageDisplay(Window &window, double posX, double posY,
    double width, double height) : Widget(window, posX, posY, width, height) { }

void MessageDisplay::draw()
{
    BLRoundRect roundRect(posX, posY, width, height, 5);
    window.blContext.fillRoundRect(roundRect, bgColor);
    window.blContext.setStrokeWidth(1.f);
    window.blContext.strokeRoundRect(roundRect, borderColor);

    const double lineHeight = blFont.size() + 2;

    double offsetX = 10;
    double offsetY = lineHeight;
    const double maxMessagesVisible = (height - lineHeight) / lineHeight;
    double messagesScrolled = (messages.size() - maxMessagesVisible) *
        scrollPercent;
    offsetY -= std::fmod(messagesScrolled, 1) * lineHeight;

    if (messagesScrolled < 0)
    {
        messagesScrolled = 0;
    }

    bool drawScrollbar = false;

    window.blContext.setFillStyle(textColor);
    window.blContext.clipToRect(BLRect(posX, posY, width, height));

    BLGlyphBuffer glyphBuffer;
    BLTextMetrics textMetrics;
    blFont.shape(glyphBuffer);
    glyphBuffer.setUtf8Text(" ");
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    const double spaceWidth = textMetrics.advance.x;

    // timestamp, nick, messages
    typedef std::tuple<std::string, std::string, std::vector<std::string>>
        PrintMessage;
    std::vector<PrintMessage> printMessages;

    int offsetYTest = offsetY;
    double offsetXText = offsetX;

    int nLines = 0;
    int nNewlines = 0;

    for (Message message : std::vector(messages.begin() +
        std::floor(messagesScrolled), messages.end()))
    {
        PrintMessage printMessage;

        if (nLines - nNewlines > maxMessagesVisible)
        {
            break;
        }

        tm* timeInfo = localtime(&std::get<0>(message));
        std::string msgText('[' + std::to_string(timeInfo->tm_hour) + ':' +
            std::to_string(timeInfo->tm_min) + ':' +
            std::to_string(timeInfo->tm_sec) + ']');
        glyphBuffer.setUtf8Text(msgText.c_str());
        blFont.getTextMetrics(glyphBuffer, textMetrics);
        std::get<0>(printMessage) = msgText;

        double nextPosXCandidate = posX + offsetXText + textMetrics.advance.x +
            10;

        if (nextPosXCandidate > nickPosX)
        {
            nickPosX = nextPosXCandidate;
        }
        else
        {
            nextPosXCandidate = nickPosX;
        }

        msgText = std::string('<' + std::get<1>(message) + '>');
        glyphBuffer.setUtf8Text(msgText.c_str());
        blFont.getTextMetrics(glyphBuffer, textMetrics);
        std::get<1>(printMessage) = msgText.c_str();

        nextPosXCandidate += textMetrics.advance.x + 10;

        if (nextPosXCandidate > msgPosX)
        {
            msgPosX = nextPosXCandidate;
        }

        std::string word;
        std::stringstream ss(std::get<2>(message));

        if (!std::getline(ss, word, ' '))
        {
            continue;
        }

        glyphBuffer.setUtf8Text(word.c_str());
        blFont.getTextMetrics(glyphBuffer, textMetrics);

        double wordPosX = msgPosX + textMetrics.advance.x + spaceWidth;
        int msgIndex = 0;
        int lastMsgIndex = 0;
        nLines++;

        ss.str(std::get<2>(message));

        while (std::getline(ss, word, ' '))
        {
            glyphBuffer.setUtf8Text(word.c_str());
            blFont.getTextMetrics(glyphBuffer, textMetrics);

            if (wordPosX + textMetrics.advance.x >= posX + width - 5)
            {
                nLines++;
                nNewlines++;
                std::get<2>(printMessage).emplace_back(
                    std::get<2>(message).substr(lastMsgIndex, msgIndex -
                    lastMsgIndex - 1));
                lastMsgIndex = msgIndex;

                offsetYTest += lineHeight;

                wordPosX = msgPosX;
            }

            wordPosX += textMetrics.advance.x + spaceWidth;
            msgIndex += word.length() + 1;
        }

        std::get<2>(printMessage).emplace_back(std::get<2>(message).substr(
            lastMsgIndex));

        printMessages.push_back(std::move(printMessage));

        offsetYTest += lineHeight;
    }

    double wrapOverflowShift{offsetYTest > height ? (offsetYTest - height) *
        scrollPercent : 0};

    for (PrintMessage& printMessage : printMessages)
    {
        double printY = posY + offsetY - wrapOverflowShift;
        window.blContext.fillUtf8Text(BLPoint(posX + offsetX, printY), blFont,
            std::get<0>(printMessage).c_str());
        window.blContext.fillUtf8Text(BLPoint(nickPosX, printY), blFont,
            std::get<1>(printMessage).c_str());

        for (std::string& message : std::get<2>(printMessage))
        {
            window.blContext.fillUtf8Text(BLPoint(msgPosX, printY), blFont,
                message.c_str());
            printY += lineHeight;
            offsetY += lineHeight;
        }
    }

    if (nLines > std::floor(maxMessagesVisible))
    {
        double scrollbarLen = height * maxMessagesVisible /
            (double)messages.size();
        double scrollPosY = posY + scrollPercent * (height - scrollbarLen);
        BLLine scrollLine(posX + width - 7, scrollPosY, posX + width - 7,
            scrollPosY + scrollbarLen);
        window.blContext.setStrokeWidth(5);
        window.blContext.strokeLine(scrollLine, textColor);
    }

    window.blContext.restoreClipping();
}

void MessageDisplay::logMessage(Message message)
{
    messages.push_back(std::move(message));
}

void MessageDisplay::scroll(double distance)
{
    const double lineHeight = blFont.size() + 2;
    const double maxMessagesVisible = (height - lineHeight) / lineHeight;
    const double scrollableDistance = ((double)messages.size() -
        maxMessagesVisible) * lineHeight;

    if (scrollableDistance <= 0)
    {
        return;
    }

    scrollPercent += distance / scrollableDistance;

    if (scrollPercent < 0)
    {
        scrollPercent = 0;
    }
    else if (scrollPercent > 1)
    {
        scrollPercent = 1;
    }
}

Tab::Tab(Window& window, double posX, double posY, double width, double height,
    std::string name, TabBar& tabBar)
    : Selectable(window, posX, posY, width, height)
    , name{name}
    , tabBar{tabBar} { }

void Tab::draw()
{
    BLRoundRect roundRect{posX, posY, width, height + 5, 10};
    window.blContext.clipToRect(BLRect(posX, posY, width, height));

    window.blContext.fillRoundRect(roundRect, tabBar.activeTab->first.get() ==
        this ? activeColor : bgColor);
    window.blContext.setStrokeWidth(1);
    window.blContext.strokeRoundRect(roundRect, borderColor);

    BLGlyphBuffer glyphBuffer;
    BLTextMetrics textMetrics;
    blFont.shape(glyphBuffer);
    glyphBuffer.setUtf8Text(name.c_str());
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    BLPoint textPos{(width - textMetrics.advance.x) / 2 + posX, posY + height -
        (height - blFont.size()) / 2 - 2};
    window.blContext.fillUtf8Text(textPos, blFont, name.c_str());

    window.blContext.restoreClipping();
}

void Tab::select()
{
    try
    {
        tabBar.activeTab = &tabBar.messageDisplays.at(name);
    }
    catch (std::exception& e)
    {
        tabBar.activeTab = nullptr;
    }
}

TabBar::TabBar(Window& window, double posX, double posY, double width, double
    height) : Widget(window, posX, posY, width, height)
{
    addChannel("global");
    activeTab = &messageDisplays.at("global");
}

void TabBar::draw()
{
    std::for_each(messageDisplays.begin(), messageDisplays.end(), [&](auto& x){
        x.second.first->draw(); });

    if (activeTab)
    {
        activeTab->second.draw();
    }
}

void TabBar::addChannel(const std::string& name)
{
    messageDisplays.emplace(name, std::make_pair(std::make_unique<Tab>(window,
        tabX, posY, 100, height, name, *this), MessageDisplay(window, posX,
        posY + height, width, 500)));
    tabX += 100;
}

void TabBar::closeTab(const std::string& channel)
{
    auto messageDisplay = messageDisplays.find(channel);

    if (messageDisplay == messageDisplays.end())
    {
        return;
    }

    double gapPos = messageDisplay->second.first->posX;
    double gapWidth = messageDisplay->second.first->width;

    if (&messageDisplay->second == activeTab)
    {
        activeTab = &messageDisplays.at("global");
    }

    messageDisplays.erase(messageDisplay);

    // shift proceeding tabs to fill empty space
    for (auto tab = messageDisplays.begin(); tab != messageDisplays.end();
        ++tab)
    {
        if (tab->second.first->posX > gapPos)
        {
            tab->second.first->posX -= gapWidth;
        }
    }

    tabX -= gapWidth;
}
