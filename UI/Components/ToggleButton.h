#pragma once

#include "Button.h"

class ToggleButton : public Button {
private:
    bool& var; // Reference to an external boolean variable
    bool toggled = false;

public:
    ToggleButton(Rectangle rec, Color color, int fontSize, std::string text, bool& var) :
        Button(rec, color, fontSize, text),
        var(var)
    {
    }

    ~ToggleButton() = default;

    void draw() const override
    {
        Color drawColor = toggled ? color : PINK;
        DrawRectangleRec(rec, drawColor);

        drawText();
    }

    void onClick(Vector2 mousePos) override
    {
        if (isHovered(mousePos))
        {
            toggled = !toggled;
            var = toggled;
        }
    }
};