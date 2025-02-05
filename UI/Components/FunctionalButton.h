#pragma once

#include <functional>
#include <iostream>
#include "Button.h"

class FunctionalButton : public Button
{
private:
	std::function<void()> function;

public:
	FunctionalButton(Rectangle rec, Color color, int fontSize, std::string text, std::function<void()> func) :
		Button (rec, color, fontSize, text ),
		function (func)
	{
	}

	~FunctionalButton() = default;

	void onClick(Vector2 mousePos) override
	{
		if (isHovered(mousePos))
		{
			function();
		}
	}
};