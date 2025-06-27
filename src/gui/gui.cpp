#include "gui.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <blend2d.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

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
    if (!fcMatch || !FcPatternGetString(fcMatch, FC_FILE, 0, &fontfile)
        == FcResultMatch || !fontfile)
    {
        throw GuiError("failed to locate font");
    }
    std::string fontPath = (char*)fontfile;
    FcPatternDestroy(fcPattern);
    FcPatternDestroy(fcMatch);
    #endif

    BLResult fontLoadResult =
        gui::blFontFace.createFromFile(std::move(fontPath.c_str()));
    
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
    double height, std::string label, std::function<void()> activate)
    : Selectable(window, posX, posY, width, height, SelectType::BUTTON)
    , label{label}
    , activate{std::move(activate)} { }

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
    textBuffer += std::move(input);
}

void TextBox::eraseChar()
{
    textBuffer.pop_back();
}

MessageDisplay::MessageDisplay(Window& window, double posX, double posY,
    double width, double height) : Widget(window, posX, posY, width, height) { }

void MessageDisplay::draw()
{
    BLRoundRect roundRect(posX, posY, width, height, 5);
    window.blContext.fillRoundRect(roundRect, bgColor);
    window.blContext.setStrokeWidth(1.f);
    window.blContext.strokeRoundRect(roundRect, borderColor);

    int offsetX = 20;
    int offsetY = 20;

    window.blContext.setFillStyle(textColor);

    for (Message message : messages)
    {
        window.blContext.fillUtf8Text(BLPoint(posX + offsetX, posY + offsetY),
            blFont, ('<' + std::get<1>(message) + "> "
            + std::get<2>(message)).c_str());
        offsetY += blFont.size() + 2;
    }
}

void MessageDisplay::logMessage(Message message)
{
    messages.push_back(std::move(message));
}