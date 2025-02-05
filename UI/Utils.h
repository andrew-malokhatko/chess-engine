#pragma once

#include "raylib.h"

namespace Utils
{
	bool inRange(int x, int y);
	bool inRange(int index);
	int toIndex(int x, int y);
	int toIndex(Vector2 position);
	Vector2 toPosition(int position);

	Color mixColors(Color base, Color highlight, float alpha);
}