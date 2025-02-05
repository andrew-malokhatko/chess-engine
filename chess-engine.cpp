#include "Game.h"

int main()
{
	Game game = Game();

	while (game.isRunning())
	{
		game.update();
		game.draw();
	}
}

