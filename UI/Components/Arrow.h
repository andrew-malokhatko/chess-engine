#pragma once

#include <raylib.h>

class Arrow
{
	static constexpr float thick = 12.5f;
	static int squareSize;
	static Vector2 offset;

	Color color;
	int start;
	int end;

public:
	static void setConstants(int squareSize, Vector2 offset);
	Arrow(int start, int end, Color color = RED);
	void draw() const;
};