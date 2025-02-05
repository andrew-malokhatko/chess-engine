#pragma once

#include <string>
#include <raylib.h>

#include "Clipboard/Clipboard.h"

class Input
{
	Color color;
	int fontSize;
	std::string text;

	bool selected = false;

public:
	Rectangle rec;

	Input(Rectangle rec, Color color, int fontSize, std::string text) :
		rec {rec},
		color { color },
		fontSize { fontSize },
		text { text }
	{
	}

	void onClick(Vector2 mousePosition)
	{
		if (CheckCollisionPointRec(mousePosition, rec))
		{
			selected = true;
		}
		else
		{
			selected = false;
		}
	}

	void update(int key)
	{
		if (!selected)
		{
			return;
		}

		if (key == KEY_BACKSPACE && !text.empty())
		{
			text.pop_back();
		}
		else if ((key >= 32) && (key <= 125)) // valid key range
		{
			text.push_back(key);
		}
	}

	std::string submit()
	{
		if (selected)
			return text;

		return "";
	}

	void ctrV()
	{
		text = Clipboard::getBuffer();
	}

	void draw()
	{
		const int textWidth = MeasureText(text.c_str(), fontSize);

		int textX = rec.x + (rec.width - textWidth) / 2;
		int textY = rec.y + (rec.height - fontSize) / 2;

		DrawRectangle(rec.x, rec.y, rec.width, rec.height, color);
		DrawText(text.c_str(), textX, textY, fontSize, BLACK);
	}
};