#pragma once

#include <blend2d.h>
#include <exception>
#include <memory>
#include <string>
#include <SDL3/SDL.h>
#include <vector>
#include <functional>
#include <ctime>

namespace gui
{
    class GuiError;
    class Window;
    class Widget;
    class Selectable;
    class Button;
    class TextBox;
    class MessageDisplay;
    class Tab;
    class TabBar;

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
        Widget(Window& window, double posX, double posY, double width,
            double height);
        double posX;
        double posY;
        double width;
        double height;
        virtual void draw() = 0;
    };

    class Selectable : public Widget
    {
        static std::vector<Selectable*> existing;
    public:
        enum SelectType { NONE, BUTTON, TEXT_BOX };
        SelectType selectType;
        Selectable(Window& window, double posX, double posY, double width,
            double height, SelectType selectType = SelectType::NONE);
        ~Selectable();

        static void findFocus(double mouseX, double mouseY);
        static Selectable* hovered;
        static Selectable* selected;
        virtual void select() = 0;
    };

    class Button : public Selectable
    {
        std::string label;
    public:
        std::function<void()> activate;
        BLRgba32 bgColor = BLRgba32(0xff454662);
        BLRgba32 borderColor = BLRgba32(0xff686881);
        BLRgba32 textColor = BLRgba32(0xffffffff);

        void draw(bool highlight);
        Button(Window& window, double posX, double posY, double width,
            double height, std::string label, std::function<void()>&& activate);
        void draw() override;

        void select() override;
    };

    class TextBox : public Selectable
    {
    public:
        TextBox(Window& window, double posX, double posY, double width,
            double height);
        std::string textBuffer;
        void draw() override;
        void draw(bool highlight);
        BLRgba32 bgColor = BLRgba32(0xff000000);
        BLRgba32 highlightColor = BLRgba32(0xff404040);
        BLRgba32 borderColor = BLRgba32(0xffffffff);
        BLRgba32 textColor = BLRgba32(0xffffffff);

        void select() override;
        void writeChar(char input);
        void eraseChar();
    };

    class MessageDisplay : public Widget
    {
    public:
        // time logged, nick, message
        typedef std::tuple<std::time_t, std::string, std::string> Message;
    private:
        std::vector<Message> messages;
        double nickPosX = 0;
        double msgPosX = 0;
    public:
        double scrollPercent = 1;
        MessageDisplay(Window& window, double posX, double posY, double width,
            double height);
        BLRgba32 bgColor = BLRgba32(0xff000000);
        BLRgba32 highlightColor = BLRgba32(0xff404040);
        BLRgba32 borderColor = BLRgba32(0xffffffff);
        BLRgba32 textColor = BLRgba32(0xffffffff);

        void draw() override;
        void logMessage(Message message);
        void scroll(double distance);
    };

    class Tab : public Selectable
    {
        double tabHeight;
        std::string name;
        TabBar& tabBar;
    public:
        const std::string& getName{name};
        BLRgba32 bgColor{BLRgba32(0xff353652)};
        BLRgba32 borderColor{BLRgba32(0xff686881)};
        BLRgba32 textColor{BLRgba32(0xffffffff)};
        BLRgba32 activeColor{BLRgba32(0xff454662)};
        Tab(Window& window, double posX, double posY, double width, double
            height, std::string name, TabBar& tabBar);
        void draw() override;
        void select() override;
    };

    class TabBar : public Widget
    {
        double tabX{posX + 5};
    public:
        std::pair<std::unique_ptr<Tab>, MessageDisplay>* activeTab = nullptr;
        std::unordered_map<std::string, std::pair<std::unique_ptr<Tab>,
            MessageDisplay>> messageDisplays;
        TabBar(Window& window, double posX, double posY, double width, double
            height);

        void draw() override;
        void addChannel(const std::string& name);
    };

    static BLFont blFont;
    static BLFontFace blFontFace;

    char readChar(const SDL_Event& event, bool shiftKey);
}
