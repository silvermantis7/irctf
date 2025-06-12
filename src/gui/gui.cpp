#include "gui.hpp"
#include <SDL3/SDL_render.h>
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

void Window::draw(Widget& target)
{
    target.draw(blContext);
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

bool Window::isOpen()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            return false;
        }
    }

    return true;
}

Widget::Widget(double posX, double posY) : posX(posX), posY(posY) { }

Button::Button(double posX, double posY, double width, double height,
    std::string label, std::function<void()> purpose)
    : Widget(posX, posY)
    , width(width)
    , height(height)
    , label(label)
    , purpose(std::move(purpose)) { }

void Button::draw(BLContext& context)
{
    const auto p1 = std::chrono::system_clock::now();
    auto offset = std::chrono::duration_cast<std::chrono::milliseconds>
        (p1.time_since_epoch()).count();

    BLRoundRect roundRect(posX, posY, width, height, 5);
    context.strokeRoundRect(roundRect);
    context.fillRoundRect(roundRect, BLRgba32(0xff454662));
    context.setStrokeWidth(1.5);
    context.strokeRoundRect(roundRect, BLRgba32(0xff686881));

    BLGlyphBuffer glyphBuffer;
    BLTextMetrics textMetrics;
    glyphBuffer.setUtf8Text(label.c_str());
    blFont.shape(glyphBuffer);
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    double textWidth = textMetrics.boundingBox.x1 - textMetrics.boundingBox.x0;
    double textHeight = blFont.metrics().ascent - blFont.metrics().descent;

    context.setFillStyle(BLRgba32(0xffffffff));
    context.fillUtf8Text(BLPoint(posX + width / 2.f - textWidth / 2.f, posY
        + height - (height - textHeight) / 2.f), blFont, label.c_str());
}