#pragma once

#include "Button.h"
#include <functional>
#include <vector>


template <typename EnumType>
class ToggleTextButton : public Button
{
private:

	struct EnumState
	{
		EnumType value;
		std::string text;
	};

	std::vector<EnumState> states;
	std::function<void(EnumType)> setter;
	size_t index = 0;

public:
	ToggleTextButton(Rectangle rec, Color color, int fontSize, std::vector<EnumState> states, std::function<void(EnumType)> setter) :
		Button(rec, color, fontSize, ""),
		states { states },
		setter { setter }
	{
		if (!states.empty())
		{
			text = states[0].text;
		}
	}

	void onClick(Vector2 mousePosition) override
	{
		if (!isHovered(mousePosition) || states.empty())
		{
			return;
		}

		index = (index + 1) % states.size();
		text = states[index].text;
		setter(states[index].value);
	}
};