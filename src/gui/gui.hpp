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
        public:
        Window(int width, int height, std::string title);
        ~Window();
        BLContext blContext;
        void clear();
        void display();
        bool pollEvents(SDL_Event& event);
    };

    class Widget
    {
    protected:
        Window& window;
    public:
        Widget(Window& window, double posX, double posY);
        double posX;
        double posY;
        virtual void draw() = 0;
    };

    class Button : public Widget
    {
        std::string label;
        public:
        std::function<void()> activate;
        uint32_t bgColor = 0xff454662;
        uint32_t borderColor = 0xff686881;
        uint32_t textColor = 0xffffffff;

        static BLFont blFont;
        static BLFontFace blFontFace;
        double width;
        double height;
        void draw(bool highlight);
        Button(Window& window, double posX, double posY, double width,
            double height, std::string label, std::function<void()> activate);
        void draw() override;
        bool mouseOver();
    };
}