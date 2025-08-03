#include "log_item.hpp"
#include "../gui.hpp"
#include <sstream>
#include <string>
#include <blend2d.h>

using namespace gui;
using namespace gui::log_item;

void MessageDisplay::formatMessage(
    Message* message,
    int* nLines,
    int* nNewlines,
    const double* maxMessagesVisible,
    const double* offsetXText,
    const double* spaceWidth,
    int* offsetYTest,
    const double* lineHeight
) {
    BLGlyphBuffer glyphBuffer;
    BLTextMetrics textMetrics;

    // format time logged
    tm* timeInfo = localtime(&message->timeLogged);
    message->formatted.timeLogged =
        '['
        + std::to_string(timeInfo->tm_hour)
        + ':'
        + std::to_string(timeInfo->tm_min)
        + ':'
        + std::to_string(timeInfo->tm_sec)
        + ']';
    blFont.shape(glyphBuffer);
    glyphBuffer.setUtf8Text(message->formatted.timeLogged.c_str());
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    double nextPosXCandidate {
        posX
        + *offsetXText
        + textMetrics.advance.x
        + 10.f
    };

    // update nick column x position
    if (nextPosXCandidate > nickPosX)
    {
        nickPosX = nextPosXCandidate;
    }
    else
    {
        nextPosXCandidate = nickPosX;
    }

    // enclose nick in angle brackets
    message->formatted.nick = std::string('<' + message->nick + '>');
    glyphBuffer.setUtf8Text(message->formatted.nick.c_str());
    blFont.getTextMetrics(glyphBuffer, textMetrics);

    nextPosXCandidate += textMetrics.advance.x + 10;

    // update message column x position
    if (nextPosXCandidate > msgPosX)
    {
        msgPosX = nextPosXCandidate;
    }

    std::string word;
    std::stringstream ss(message->rawMessage);

    double nextWordPosX { msgPosX + textMetrics.advance.x + *spaceWidth };
    int msgIndex = 0;
    int lastMsgIndex = 0;
    ++(*nLines);

    // re-initialize stringstream to reset std::getline
    ss.str(message->rawMessage);

    // clear message lines from previous call to this method
    message->formatted.messageLines.clear();

    // for every word in single-line message
    while (std::getline(ss, word, ' '))
    {
        glyphBuffer.setUtf8Text(word.c_str());
        blFont.getTextMetrics(glyphBuffer, textMetrics);
        nextWordPosX += textMetrics.advance.x + *spaceWidth;

        // if word reaches right side of message display
        if (nextWordPosX >= posX + width - 5)
        {
            ++(*nLines);
            ++(*nNewlines);

            // add line to formatted message
            message->formatted.messageLines.emplace_back(
                message->rawMessage.substr(
                    lastMsgIndex,
                    msgIndex - lastMsgIndex - 1
                )
            );
            lastMsgIndex = msgIndex;

            // set next word coordinates to start of new line
            *offsetYTest += *lineHeight;
            nextWordPosX = msgPosX;
        }

        msgIndex += word.length() + 1;
    }

    // add final line to formatted message
    message->formatted.messageLines.emplace_back(
        message->rawMessage.substr(lastMsgIndex)
    );

    *offsetYTest += *lineHeight;
}

void gui::MessageDisplay::drawItem(
    FormattedMessage* message,
    double* offsetY,
    const double* wrapOverflowShift,
    const double* offsetX,
    const double* lineHeight
) {
    double printY { posY + *offsetY - *wrapOverflowShift };

    // draw time logged
    window.blContext.fillUtf8Text(
        BLPoint(posX + *offsetX, printY),
        blFont,
        message->timeLogged.data()
    );

    // draw nick
    window.blContext.fillUtf8Text(
        BLPoint(nickPosX, printY),
        blFont,
        message->nick.data()
    );

    // draw each message line
    for (std::string& messageLine : message->messageLines)
    {
        window.blContext.fillUtf8Text(
            BLPoint(msgPosX, printY),
            blFont,
            messageLine.data()
        );

        printY += *lineHeight;
        *offsetY += *lineHeight;
    }
}

void gui::MessageDisplay::drawItem(
    const Join* join,
    const double* offsetX,
    double* offsetY,
    const double* wrapOverflowShift,
    const double* lineHeight
) {
    const double textHeight { blFont.metrics().ascent };
    const double drawY { posY + *offsetY - textHeight + 3 };
    const double drawX { posX + *offsetX };

    const BLPoint leftArrow[] = {
        BLPoint(drawX, drawY + textHeight / 2),
        BLPoint(drawX + 8, drawY),
        BLPoint(drawX + 8, drawY + textHeight * 0.3),
        BLPoint(drawX + 20, drawY + textHeight * 0.3),
        BLPoint(drawX + 20, drawY + textHeight * 0.7),
        BLPoint(drawX + 8, drawY + textHeight * 0.7),
        BLPoint(drawX + 8, drawY + textHeight)
    };

    window.blContext.fillPolygon(
        leftArrow,
        sizeof(leftArrow) / sizeof(BLPoint)
    );

    window.blContext.fillUtf8Text(
        BLPoint(drawX + 25, posY + *offsetY - *wrapOverflowShift),
        blFont,
        join->user.c_str()
    );

    *offsetY += *lineHeight;
}

void gui::MessageDisplay::drawItem(
    const Part* part,
    const double* offsetX,
    double* offsetY,
    const double* wrapOverflowShift,
    const double* lineHeight
) {
    const double textHeight { blFont.metrics().ascent };
    const double drawY { posY + *offsetY - textHeight + 3 };
    const double drawX { posX + *offsetX };
    const double textY { posY + *offsetY - *wrapOverflowShift };

    const BLPoint rightArrow[] = {
        BLPoint(drawX + 20, drawY + textHeight / 2),
        BLPoint(drawX + 12, drawY),
        BLPoint(drawX + 12, drawY + textHeight * 0.3),
        BLPoint(drawX, drawY + textHeight * 0.3),
        BLPoint(drawX, drawY + textHeight * 0.7),
        BLPoint(drawX + 12, drawY + textHeight * 0.7),
        BLPoint(drawX + 12, drawY + textHeight)
    };

    window.blContext.fillPolygon(
        rightArrow,
        sizeof(rightArrow) / sizeof(BLPoint)
    );

    const double nickPosX { drawX + 25 };

    window.blContext.fillUtf8Text(
        BLPoint(nickPosX, textY),
        blFont,
        part->user.c_str()
    );

    if (part->message.has_value())
    {
        BLTextMetrics textMetrics;
        BLGlyphBuffer glyphBuffer;
        blFont.shape(glyphBuffer);
        glyphBuffer.setUtf8Text(part->user.c_str());
        blFont.getTextMetrics(glyphBuffer, textMetrics);

        window.blContext.setStrokeWidth(2);

        window.blContext.strokeLine(
            BLPoint(nickPosX + textMetrics.advance.x + 7, drawY),
            BLPoint(nickPosX + textMetrics.advance.x + 7, drawY + textHeight),
            textColor
        );

        window.blContext.fillUtf8Text(
            BLPoint(nickPosX + textMetrics.advance.x + 13, textY),
            blFont,
            part->message.value().c_str()
        );
    }

    *offsetY += *lineHeight;
}