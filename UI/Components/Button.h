#pragma once

#include <raylib.h>
#include <string>

class Button
{
protected:
    Color color;
    int fontSize;
    std::string text;

public:
    Rectangle rec;

    Button(Rectangle rec, Color color, int fontSize, std::string text) :
        rec(rec),
        color(color),
        fontSize(fontSize),
        text(std::move(text))
    {
    }

    virtual ~Button() = default;

    virtual void drawText() const
    {
        int textWidth = MeasureText(text.c_str(), fontSize);

        int posX = rec.x + ((rec.width - textWidth) / 2);
        int posY = rec.y + ((rec.height - fontSize) / 2);

        DrawText(text.c_str(), posX, posY, fontSize, BLACK);
    }

    virtual void draw() const
    {
        DrawRectangleRec(rec, color);
        drawText();
    }

    virtual void onClick(Vector2 mousePos) = 0; // Must be overridden in derived classes

    bool isHovered(Vector2 mousePos) const
    {
        return CheckCollisionPointRec(mousePos, rec);
    }
};