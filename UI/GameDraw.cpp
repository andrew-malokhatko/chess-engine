#include "Game.h"


void Game::draw()
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	drawSearchData();
	drawTime();
	drawGameOver();
	drawEvaluation();

	if (flipped)
	{
		int a = 2;
		int b = a + 2;
	}

	for (int r = 0; r < 8; r++)	// row
	{
		for (int c = 0; c < 8; c++) // col
		{
			// use clever trick with xor to flip the col and row
			int col = flipped ? c ^ 7 : c;
			int row = flipped ? r ^ 7 : r;

			// keep c and r as posX and posY for board to actually flip and not just draw in reverse
			int posX = c * SquareSize + offsetX;
			int posY = r * SquareSize + offsetY;

			// Draw Board
			Color cellColor = getCellColor(col, row);
			DrawRectangle(posX, posY, SquareSize, SquareSize, cellColor);

			// Draw Pieces
			Chess::Piece curPiece = gameData.board[Utils::toIndex(col, row)];
			if (curPiece != Chess::Piece::None)
			{
				DrawTexture(pieceTextures[curPiece], posX, posY, WHITE);
			}
		}
	}

	// Draw Arrows
	if (displayBookMoves)
	{
		for (const Arrow& bookArrow : bookMoves)
		{
			bookArrow.draw();
		}
	}

	// Draw buttons
	for (auto& button : buttons)
	{
		button->draw();
	}

	// Draw Input
	fenInput.draw();

	EndDrawing();
}

uint64_t getMovesAsBits(uint64_t square, const std::vector<Chess::Move>& legalMoves)
{
	uint64_t moves = 0ULL;

	for (const Chess::Move& move : legalMoves)
	{
		uint64_t moveMask = 1ULL << move.to;
		moves |= moveMask;
	}

	return moves;
}

Color Game::getCellColor(int col, int row)
{
	const float blendPower = 0.5;
	const int settingsSize = 6;

	const bool drawSettings[] = { displayThreatMapWhite, displayThreatMapBlack, displayCheckMaskWhite,
									displayCheckMaskBlack, displayPinMaskWhite, displayPinMaskBlack };

	const uint64_t drawMasks[] = { debugData.masksWhite.threatmap, debugData.masksBlack.threatmap, debugData.masksWhite.checkMask,
									debugData.masksBlack.checkMask, debugData.masksWhite.pinMask, debugData.masksBlack.pinMask };

	const Color defaultColors[] = { WHITE, SKYBLUE };
	const Color secondaryColors[] = { MAROON, MAROON, DARKBLUE, DARKBLUE, ORANGE, ORANGE };

	int squareIndex = Utils::toIndex(col, row);
	uint64_t square = 1ULL << squareIndex;
	uint64_t fromSelectedSquare = getMovesAsBits(square, movesFromSelectedSquare);


	int even = (col + row) % 2;
	Color base = defaultColors[even];

	Color secondary = base;

	for (int i = 0; i < settingsSize; i++)
	{
		if (!drawSettings[i]) continue;

		if (drawMasks[i] & square)
		{
			secondary = secondaryColors[i];
		}
	}

	secondary = selectedSquare == squareIndex ? RED : secondary;
	secondary = fromSelectedSquare & square ? GREEN : secondary;

	return Utils::mixColors(base, secondary, blendPower);
}

void Game::drawEvaluation()
{
	const int width = 40;
	const int fieldSize = SquareSize * 8;
	const int maxEval = 10;
	const int fontSize = 14;

	float evaluation = gameManager.getEvaluation();
	if (evaluation > maxEval) evaluation = maxEval;
	if (evaluation < -maxEval) evaluation = -maxEval;

	float whiteHeight = ((maxEval + evaluation) / (maxEval * 2)) * fieldSize;
	int whitePosY = offsetY + (fieldSize - whiteHeight);
	int posX = offsetX - width - 10;

	DrawRectangle(posX, offsetY, width, fieldSize, BLACK);
	DrawRectangle(posX, whitePosY, width, whiteHeight, WHITE);

	int textPosY = evaluation < 0 ? offsetY + 10 : offsetY + fieldSize - 20;
	Color textColor = evaluation < 0 ? WHITE : BLACK;

	std::string evalText = std::format("{:.2f}", evaluation);
	int evalTextWidth = MeasureText(evalText.c_str(), fontSize);
	DrawText(evalText.c_str(), posX + ((width - evalTextWidth) / 2), textPosY, 14, textColor);
}


void Game::drawGameOver()
{
	const int fontSize = 60;

	Chess::GameState gameState = gameData.gameState;

	int posX = offsetX + SquareSize * 8.5;
	int posY = (WindowHeight / 2) - (fontSize / 2);

	if (gameState.isGameOver())
	{
		std::string text = gameState.whiteWon() ? "White Won" : gameState.blackWon() ? "Black Won" : "Stalemate";

		DrawText(text.c_str(), posX, posY, fontSize, GREEN);
		//DrawText("(Press \"Enter\" to restart)", offsetX + SquareSize * 8.1, (WindowHeight / 2) + SquareSize * 0.7, 28, GREEN);
	}
	else
	{
		DrawRectangle(posX, posY, SquareSize * 3, 10, SKYBLUE);
	}
}


void Game::drawTime()
{
	const int TextSize = 90;
	const int TextWidth = MeasureText("00:00", TextSize);

	const Color BaseColor = SKYBLUE;
	const Color GameOverColor = GRAY;

	Chess::GameState gameState = gameData.gameState;

	Color textColor = gameState.isGameOver() ? GameOverColor : BaseColor;

	std::string blackMinutes = std::format("{:02}", std::chrono::duration_cast<std::chrono::minutes>(gameData.timeBlack).count());
	std::string whiteMinutes = std::format("{:02}", std::chrono::duration_cast<std::chrono::minutes>(gameData.timeWhite).count());

	std::string blackSeconds = std::format("{:02}", std::chrono::duration_cast<std::chrono::seconds>(gameData.timeBlack).count() % 60);
	std::string whiteSeconds = std::format("{:02}", std::chrono::duration_cast<std::chrono::seconds>(gameData.timeWhite).count() % 60);

	int posX = offsetX + SquareSize * 9;
	int posY = offsetY + SquareSize * 1.5;

	DrawText((blackMinutes + ":" + blackSeconds).c_str(), posX, posY, TextSize, textColor);
	DrawText((whiteMinutes + ":" + whiteSeconds).c_str(), posX, WindowHeight - posY - TextSize, TextSize, textColor);
}

Texture2D Game::toCroppedTexture(int col, int row)
{
	Image pieces = ImageCopy(assets.piecesImage);
	Rectangle cropRect = { TextureSize * col, TextureSize * row,TextureSize, TextureSize };

	ImageCrop(&pieces, cropRect);
	ImageResize(&pieces, SquareSize, SquareSize);

	Texture2D result = LoadTextureFromImage(pieces);

	UnloadImage(pieces);

	return result;
}

void Game::loadTextures()
{
	for (int i = Chess::Piece::WhiteRook; i <= Chess::Piece::WhitePawn; i++)
	{
		pieceTextures[Chess::Piece(i)] = toCroppedTexture(i - Chess::Piece::WhiteRook, 0);
	}

	for (int i = Chess::Piece::BlackRook; i <= Chess::Piece::BlackPawn; i++)
	{
		pieceTextures[Chess::Piece(i)] = toCroppedTexture(i - Chess::Piece::BlackRook, 1);
	}
}

void Game::drawSearchData()
{
	const int posX = buttonsX;
	const int posY = offsetY + SquareSize * 6;
	const int width = 220;
	const int height = 35;
	const int statSize = 5;
	const int fontSize = 20;

	Chess::SearchStats searchStats = debugData.searchStats;

	const Color colors[] = { SKYBLUE, GREEN, GOLD, ORANGE, PURPLE };
	const int stats[] = { searchStats.depth, searchStats.nodesEvaluated, searchStats.nodesPruned,
						searchStats.nodesTransposed, searchStats.nodesVisited };


	const char* captions[] = { "Depth: ", "Evaluated: ", "Pruned: ", "Transosed: ", "Visited: "};

	for (int i = 0; i < statSize; i++)
	{
		std::string text = std::string(captions[i] + std::to_string(stats[i]));
		DrawText(text.c_str(), posX, posY + (height * i), fontSize, colors[i]);
	}
}