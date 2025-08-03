#pragma once

#include <blend2d.h>
#include <exception>
#include <memory>
#include <string>
#include <SDL3/SDL.h>
#include <vector>
#include <functional>
#include <ctime>

#include "gui/log_item.hpp"

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
        std::vector<log_item::LogItem> messages;
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
        void logMessage(log_item::LogItem&& logItem);
        void scroll(double distance);

        void formatMessage(
            log_item::Message* message,
            int* nLines,
            int* nNewlines,
            const double* maxMessagesVisible,
            const double* offsetXText,
            const double* spaceWidth,
            int* offsetYTest,
            const double* lineHeight
        );

        void drawItem(
            log_item::FormattedMessage* message,
            double* offsetY,
            const double* wrapOverflowShift,
            const double* offsetX,
            const double* lineHeight
        );

        void drawItem(
            const log_item::Join* join,
            const double* offsetX,
            double* offsetY,
            const double* wrapOverflowShift,
            const double* lineHeight
        );

        void drawItem(
            const log_item::Part* part,
            const double* offsetX,
            double* offsetY,
            const double* wrapOverflowShift,
            const double* lineHeight
        );
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
        void closeTab(const std::string& channel);
    };

    inline BLFont blFont;
    inline BLFontFace blFontFace;

    char readChar(const SDL_Event& event, bool shiftKey);
}
