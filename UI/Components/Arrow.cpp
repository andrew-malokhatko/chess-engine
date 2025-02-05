#include "Arrow.h"

int Arrow::squareSize = 0;
Vector2 Arrow::offset = Vector2{ 0, 0 };	

Arrow::Arrow(int start, int end, Color color) :
	start { start },
	end { end },
	color { color }
{
}

void Arrow::setConstants(int squareSize, Vector2 offset)
{
	Arrow::squareSize = squareSize;
	Arrow::offset = offset;
}

void Arrow::draw() const
{
	int startRow = start / 8;
	int startCol = start % 8;
	int endRow = end / 8;
	int endCol = end % 8;

	float startX = startCol * squareSize + offset.x + squareSize / 2;
	float startY = startRow * squareSize + offset.y + squareSize / 2;

	float endX = endCol * squareSize + offset.x + squareSize / 2; 
	float endY = endRow * squareSize + offset.y + squareSize / 2;

	DrawLineEx(Vector2{ startX, startY }, Vector2{ endX, endY }, thick, color);
}