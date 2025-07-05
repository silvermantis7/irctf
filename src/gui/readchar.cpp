#include "gui.hpp"

char gui::readChar(const SDL_Event& event, bool shiftKey)
{
    char letter = '\0';

    switch (event.key.key)
    {
    case SDLK_SPACE:
        letter = ' ';
        break;
    case SDLK_A:
        letter = 'a';
        break;
    case SDLK_B:
        letter = 'b';
        break;
    case SDLK_C:
        letter = 'c';
        break;
    case SDLK_D:
        letter = 'd';
        break;
    case SDLK_E:
        letter = 'e';
        break;
    case SDLK_F:
        letter = 'f';
        break;
    case SDLK_G:
        letter = 'g';
        break;
    case SDLK_H:
        letter = 'h';
        break;
    case SDLK_I:
        letter = 'i';
        break;
    case SDLK_J:
        letter = 'j';
        break;
    case SDLK_K:
        letter = 'k';
        break;
    case SDLK_L:
        letter = 'l';
        break;
    case SDLK_M:
        letter = 'm';
        break;
    case SDLK_N:
        letter = 'n';
        break;
    case SDLK_O:
        letter = 'o';
        break;
    case SDLK_P:
        letter = 'p';
        break;
    case SDLK_Q:
        letter = 'q';
        break;
    case SDLK_R:
        letter = 'r';
        break;
    case SDLK_S:
        letter = 's';
        break;
    case SDLK_T:
        letter = 't';
        break;
    case SDLK_U:
        letter = 'u';
        break;
    case SDLK_V:
        letter = 'v';
        break;
    case SDLK_W:
        letter = 'w';
        break;
    case SDLK_X:
        letter = 'x';
        break;
    case SDLK_Y:
        letter = 'y';
        break;
    case SDLK_Z:
        letter = 'z';
        break;
    case SDLK_1:
        letter = '1';
        break;
    case SDLK_2:
        letter = '2';
        break;
    case SDLK_3:
        letter = '3';
        break;
    case SDLK_4:
        letter = '4';
        break;
    case SDLK_5:
        letter = '5';
        break;
    case SDLK_6:
        letter = '6';
        break;
    case SDLK_7:
        letter = '7';
        break;
    case SDLK_8:
        letter = '8';
        break;
    case SDLK_9:
        letter = '9';
        break;
    case SDLK_0:
        letter = '0';
        break;
    case SDLK_APOSTROPHE:
        letter = '\'';
        break;
    case SDLK_COMMA:
        letter = ',';
        break;
    case SDLK_MINUS:
        letter = '-';
        break;
    case SDLK_PERIOD:
        letter = '.';
        break;
    case SDLK_SLASH:
        letter = '/';
        break;
    case SDLK_SEMICOLON:
        letter = ';';
        break;
    case SDLK_EQUALS:
        letter = '=';
        break;
    case SDLK_LEFTBRACE:
        letter = '[';
        break;
    case SDLK_BACKSLASH:
        letter = '\\';
        break;
    case SDLK_RIGHTBRACE:
        letter = ']';
        break;
    case SDLK_GRAVE:
        letter = '`';
        break;
    default:
        return '\0';
    }

    if (!shiftKey)
    {
        return letter;
    }

    if (letter >= 'a' && letter <= 'z')
    {
        return letter - 0x20;
    }
    else
    {
        switch (letter)
        {
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '\'':
            return '"';
        case ',':
            return '<';
        case '-':
            return '_';
        case '.':
            return '>';
        case '/':
            return '?';
        case ';':
            return ':';
        case '=':
            return '+';
        case '[':
            return '{';
        case '\\':
            return '|';
        case ']':
            return '}';
        case '`':
            return '~';
        default:
            return letter;
        }
    }
}
