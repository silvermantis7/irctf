#include <blend2d.h>
#include <exception>
#include <memory>
#include <string>
#include <SDL3/SDL.h>
#include <vector>
#include <functional>

namespace gui
{
    class GuiError;
    class Window;
    class Widget;
    class Button;

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
        void draw(Widget& target);
        void display();

        bool isOpen();
    };

    class Widget
    {
    public:
        Widget(double posX, double posY);
        double posX;
        double posY;
        virtual void draw(BLContext& context) = 0;
    };

    class Button : public Widget
    {
        std::string label;
        std::function<void()> purpose;
    public:
        static BLFont blFont;
        static BLFontFace blFontFace;
        double width;
        double height;
        Button(double xPos, double yPos, double width, double height,
            std::string label, std::function<void()> purpose);
        void draw(BLContext& context) override;
    };
}