#include "Utils.h"
#include <cstdint>

namespace Utils
{
	bool inRange(int x, int y)
	{
		return (x >= 0 && x < 8 && y >= 0 && y < 8);
	}

	bool inRange(int index)
	{
		return (index >= 0 && index < 64);
	}

	int toIndex(int x, int y)
	{
		return x + (y * 8);
	}

	int toIndex(Vector2 position)
	{
		return (int)(position.x + (position.y * 8));
	}

	Vector2 toPosition(int position)
	{
		return Vector2{ (float)(position % 8), (float)(position / 8) };
	}


	Color mixColors(Color base, Color highlight, float alpha)
	{
		return Color
		{
			static_cast<uint8_t>((1 - alpha) * base.r + alpha * highlight.r),
			static_cast<uint8_t>((1 - alpha) * base.g + alpha * highlight.g),
			static_cast<uint8_t>((1 - alpha) * base.b + alpha * highlight.b),
			255
		};
	}
}