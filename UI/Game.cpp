#include "Game.h"

#include <string>

int Game::WindowWidth = minWindowWidth;
int Game::WindowHeight = minWindowHeight;

float Game::offsetX = (WindowWidth / 2 - 4 * SquareSize);
float Game::offsetY = (WindowHeight / 2 - 4 * SquareSize);

float Game::buttonsX = offsetX - (SquareSize * 3);

Game::Game()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	InitWindow(WindowWidth, WindowHeight, "Chess");
	SetWindowMinSize(minWindowWidth, minWindowHeight);
	resize();

	SetTargetFPS(FPS);

	loadTextures();
	updateData();

	gameManager.startNewGame();
	PlaySound(assets.startSound);
}	



Game::~Game()
{
	CloseAudioDevice();
	CloseWindow();

	for (int i = 0; i < buttons.size(); i++)
	{
		delete buttons[i];
	}
}

bool Game::isRunning()
{
	return gameRunning;
}

void Game::resize()
{
	WindowWidth = GetScreenWidth();
	WindowHeight = GetScreenHeight();

	float oldOffsetY = offsetY;

	offsetX = (WindowWidth / 2 - 4 * SquareSize);
	offsetY = (WindowHeight / 2 - 4 * SquareSize);

	buttonsX = offsetX - (SquareSize * 3);

	fenInput.rec.x = offsetX;
	fenInput.rec.y = SquareSize * 8 + offsetY + 10;

	for (auto& button : buttons)
	{
		button->rec.y -= oldOffsetY;
		button->rec.y += offsetY;

		button->rec.x = buttonsX;
	}

	Arrow::setConstants(SquareSize, Vector2{ (float)offsetX, (float)offsetY });
}

void Game::update()
{
	if (WindowShouldClose())
	{
		gameRunning = false;
		return;
	}

	if (IsWindowResized() && !IsWindowFullscreen())
	{
		resize();
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		if (!gameData.gameState.isGameOver())
		{
			gameManager.runGame();
		}

		mousePosition = GetMousePosition();
		selectedSquare = calculateSelectedSquare(mousePosition);

		makeMoveToSelectedSquare();
		movesFromSelectedSquare = getMovesFromSquare(selectedSquare);
		
		// Check if any buttons were pressed
		for (auto& button : buttons)
		{
			button->onClick(mousePosition);
		}

		// check if input was clicked on
		fenInput.onClick(mousePosition);
	}

	if (IsKeyPressed(KEY_ENTER))
	{
		std::string newFen = fenInput.submit();
		if (!newFen.empty())
		{
			gameManager.loadPositionFromFEN(newFen);
		}

		gameManager.startNewGame();
	}

	if (IsKeyDown(KEY_LEFT_CONTROL))
	{
		if (IsKeyPressed(KEY_Z))
		{
			unmakeMove();
		}
		else if (IsKeyPressed(KEY_V))
		{
			fenInput.ctrV();
		}
	}
	
	fenInput.update(GetKeyPressed());
	updateData();
}

void Game::updateData()
{
	gameManager.update();
	gameData = gameManager.getGameData();
	debugData = gameManager.getDebugData();

	loadBookMoves();
}


int Game::calculateSelectedSquare(Vector2 mousePosition)
{
	int posX = (mousePosition.x - offsetX) / SquareSize;
	int posY = (mousePosition.y - offsetY) / SquareSize;

	// clever trick once again (see draw)
	if (flipped)
	{
		posX ^= 7;
		posY ^= 7;
	}
	
	// look closer and refactor 
	if (Utils::inRange(posX, posY) && mousePosition.x > offsetX && mousePosition.y > offsetY)
	{
		return Utils::toIndex(posX, posY);
	}
	else
	{
		return -1;
	}
}  

std::vector<Chess::Move> Game::getMovesFromSquare(int square)
{
	std::vector<Chess::Move> fromSquare;

	for (int i = 0; i < gameData.moves.size(); i++)
	{ 
		Chess::Move curMove = gameData.moves[i];
		if (curMove.from == square)
		{
			fromSquare.push_back(curMove);
		}
	}

	return fromSquare;
}


void Game::loadBookMoves()
{
	bookMoves.clear();

	int moveCount = 0;
	for (const auto& move : debugData.book)
	{
		moveCount += move.second;
	}

	for (const auto& move : debugData.book)
	{
		float moveProbability = ((float)move.second / moveCount);
		Color arrowColor = Utils::mixColors(GREEN, RED, moveProbability);

		Arrow arrow = Arrow{ move.first.from, move.first.to, arrowColor };
		bookMoves.push_back(arrow);
	}
}


Sound Game::getMoveSound(Chess::Move move)
{
	if (gameData.gameState.isGameOver())
	{
		return assets.endSound;
	}
	else if (gameData.isKingInCheck)
	{
		return assets.checkSound;
	}
	else if (move.isPromotion())
	{
		return assets.promotionSound;
	}
	else if (move.isCastling())
	{
		return assets.castlingSound;
	}
	else if (gameData.gameState.isCapture())
	{
		return assets.captureSound;
	}

	return assets.moveSound;
}


void Game::makeMove(Chess::Move move)
{
	gameManager.processMove(move);

	selectedSquare = -1;
	bookMoves.clear();

	updateData();

	PlaySound(getMoveSound(move));
}

void Game::unmakeMove()
{
	gameManager.unmakeMove();
	bookMoves.clear();
	selectedSquare = -1;
}

void Game::makeMoveToSelectedSquare()
{
	bool gameOver = gameData.gameState.isGameOver();

	if (selectedSquare == -1 || gameOver)
	{
		return;
	}

	for (auto& move : movesFromSelectedSquare)
	{
		if (move.to == selectedSquare)
		{
			makeMove(move);
			return;
		}
	}
}