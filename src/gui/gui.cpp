#include "gui.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <blend2d.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace gui;

GuiError::GuiError(std::string message) : message(message) { }

const char* GuiError::what() const noexcept
{
    return message.c_str();
}

BLFont Button::blFont;
BLFontFace Button::blFontFace;

void gui::init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw GuiError("failed to initialize SDL");
    }

    BLResult result = gui::Button::blFontFace.createFromFile(
        "/usr/share/fonts/truetype/lato/Lato-Regular.ttf");

    if (result != BL_SUCCESS)
    {
        throw GuiError("failed to load font");
    }

    Button::blFont.createFromFace(Button::blFontFace, 15.f);
}

void gui::terminate()
{
    SDL_Quit();
}

Window::Window(int width, int height, std::string title)
    : window(SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL))
    , pixels(std::make_unique<std::vector<uint32_t>>(width * height))
    , width(width)
    , height(height)
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
    : window(window)
    , posX(posX)
    , posY(posY)
    , width(width)
    , height(height) { }

std::vector<Hoverable*> Hoverable::existing;
Hoverable* Hoverable::current = nullptr;

Hoverable::Hoverable(Window& window, double posX, double posY, double width,
    double height)
    : Widget(window, posX, posY, width, height)
{
    existing.push_back(this);
}

Hoverable::~Hoverable()
{
    existing.erase(std::remove(existing.begin(), existing.end(), this));
}

void Hoverable::findFocus(double mouseX, double mouseY)
{
    for (Hoverable* hoverable : existing)
    {
        if (mouseX >= hoverable->posX
            && mouseX <= (hoverable->posX + hoverable->width)
            && mouseY >= hoverable->posY
            && mouseY <= (hoverable->posY + hoverable->height))
        {
            current = hoverable;
            return;
        }
    }

    current = nullptr;
}

Button::Button(Window& window, double posX, double posY, double width,
    double height, std::string label, std::function<void()> activate)
    : Hoverable(window, posX, posY, width, height)
    , label(label)
    , activate(std::move(activate)) { }

void Button::draw()
{
    draw(Hoverable::current == this);
}

void Button::draw(bool highlight)
{
    const auto p1 = std::chrono::system_clock::now();
    auto offset = std::chrono::duration_cast<std::chrono::milliseconds>
        (p1.time_since_epoch()).count();

    BLRoundRect roundRect(posX, posY, width, height, 2);
    window.blContext.strokeRoundRect(roundRect);
    window.blContext.fillRoundRect(roundRect,
        BLRgba32(highlight ? borderColor : bgColor));
    window.blContext.setStrokeWidth(1.5);
    window.blContext.strokeRoundRect(roundRect, BLRgba32(borderColor));

    BLGlyphBuffer glyphBuffer;
    BLTextMetrics textMetrics;
    glyphBuffer.setUtf8Text(label.c_str());
    blFont.shape(glyphBuffer);
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    double textWidth = textMetrics.boundingBox.x1 - textMetrics.boundingBox.x0;
    double textHeight = blFont.metrics().ascent - blFont.metrics().descent;

    window.blContext.setFillStyle(BLRgba32(textColor));
    window.blContext.fillUtf8Text(BLPoint(posX + width / 2.f - textWidth / 2.f,
        posY + height - (height - textHeight) / 2.f), blFont, label.c_str());
}