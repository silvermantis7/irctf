#pragma once

#include <optional>
#include <string>
#include <ctime>
#include <variant>
#include <vector>

namespace gui
{
    namespace log_item
    {
        enum LogItemType
        {
            MESSAGE,
            JOIN,
            PART,
        };

        struct FormattedMessage
        {
            std::string timeLogged;
            std::string nick;
            std::vector<std::string> messageLines;
        };

        struct Message
        {
            std::time_t timeLogged;
            std::string nick;
            std::string rawMessage;
            FormattedMessage formatted;
        };

        struct Join
        {
            std::string user;
        };

        struct Part
        {
            std::string user;
            std::optional<std::string> message;
        };

        typedef std::variant<Message, Join, Part> LogItem;
    }
}