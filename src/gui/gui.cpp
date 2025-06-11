#include "gui.hpp"
#include <blend2d.h>

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
    blContext.clearAll();
}

void Window::draw()
{
    BLPath path;
    path.moveTo(26, 31);
    path.cubicTo(642, 132, 587, -136, 25, 464);
    path.cubicTo(882, 404, 144, 267, 27, 31);
    blContext.fillPath(path, BLRgba32(0xFFFFFFFF));
    blContext.setStrokeWidth(3);
    blContext.strokePath(path, BLRgba32(0xFFFF0000));
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