#include <blend2d.h>
#include <exception>
#include <memory>
#include <string>
#include <SDL3/SDL.h>
#include <vector>

namespace gui
{
    class GuiError : public std::exception
    {
        std::string message;
    public:
        GuiError(std::string message);
        const char* what() const noexcept override;
    };

    void init();
    void terminate();

    class Window
    {
        int width;
        int height;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;
        std::unique_ptr<std::vector<uint32_t>> pixels;
        BLImage blImage;
        BLContext blContext;
    public:
        Window(int width, int height, std::string title);
        ~Window();

        void clear();
        void draw();
        void display();

        bool isOpen();
    };
}