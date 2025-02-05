#pragma once

#include <raylib.h>
#include <vector>
#include <map>
#include <functional>

#include "GameManager.h"
#include "AssetManager.h"

#include "ToggleButton.h"
#include "ToggleTextButton.h"

#include "FunctionalButton.h"
#include "Button.h"
#include "Input.h"
#include "Arrow.h"
#include "Utils.h"

class Game
{
private:
	static constexpr int minWindowWidth = 1600;
	static constexpr int minWindowHeight = 900;

	static constexpr int SquareSize = 100;
	static constexpr int UniquePieces = 12;
	static constexpr float TextureSize = 132;

	static constexpr int FPS = 60;

	static constexpr int btnFont = 20;
	static constexpr int btnWidth = 220;
	static constexpr int btnHeight = 30;
	static constexpr int btnDist = 40;



	static int WindowWidth;
	static int WindowHeight;

	static float offsetX;
	static float offsetY;

	static float buttonsX;
	
	// Game
	bool gameRunning = true;

	int selectedSquare = -1;
	Vector2 mousePosition;

	// Chess
	Chess::GameManager gameManager = Chess::GameManager(Chess::GameMode::Human, Chess::TimeControl::Blitz);
	Chess::GameData gameData = gameManager.getGameData();
	Chess::DebugData debugData = gameManager.getDebugData();

	std::vector<Chess::Move> movesFromSelectedSquare;

	// Textures & Drawing
	std::map<Chess::Piece, Texture2D> pieceTextures;
	std::vector<Arrow> bookMoves;

	AssetManager assets = AssetManager();

	// Settings
	bool displayThreatMapWhite = false;
	bool displayThreatMapBlack = false;
	bool displayCheckMaskWhite = false;
	bool displayCheckMaskBlack = false;
	bool displayPinMaskWhite = false;
	bool displayPinMaskBlack = false;
	bool displayBookMoves = false;
	bool flipped = false;

	std::function<void(Chess::GameMode)> gameModeSetter = [&](Chess::GameMode mode) { gameManager.setGameMode(mode); };
	std::function<void(Chess::TimeControl)> timeControlSetter = [&](Chess::TimeControl control) { gameManager.setTimeControl(control); };

	Button* gameModeButton = new ToggleTextButton<Chess::GameMode>
	(
		Rectangle{ buttonsX, offsetY + btnDist * 2, btnWidth, btnHeight },
		GOLD,
		btnFont,
		{
			{Chess::GameMode::Human, "Human"},
			{Chess::GameMode::Computer, "Computer"},
		},
		gameModeSetter
	);

	Button* timeControlButton = new ToggleTextButton<Chess::TimeControl>
		(
			Rectangle{ buttonsX, offsetY + btnDist * 3, btnWidth, btnHeight },
			GOLD,
			btnFont,
			{
				{Chess::TimeControl::Blitz, "Blitz"},
				{Chess::TimeControl::Rapid, "Rapid"},
				{Chess::TimeControl::Bullet, "Bullet"},
			},
			timeControlSetter
		);

	std::vector<Button*> buttons = {
		new FunctionalButton { Rectangle {buttonsX, offsetY, btnWidth, btnHeight}, PURPLE, btnFont, "Undo Move", [this]() { unmakeMove(); }},
		new FunctionalButton { Rectangle {buttonsX, offsetY + btnDist, btnWidth, btnHeight}, PURPLE, btnFont, "New Game", [this]() { gameManager.startNewGame(); }},

		gameModeButton,
		timeControlButton,

		// if board was flipped while playing with computer, start new game
		new FunctionalButton { Rectangle {buttonsX, offsetY + btnDist * 4, btnWidth, btnHeight}, ORANGE, btnFont, "Flip Board", [this]() {
			flipped = !flipped;
			gameManager.setComputerWhite(flipped);
			gameManager.startNewGame();
		}},

		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 6, btnWidth, btnHeight}, BLUE, btnFont, "White ThreatMap", displayThreatMapWhite },
		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 7, btnWidth, btnHeight}, BLUE, btnFont, "Black ThreatMap", displayThreatMapBlack },
		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 8.5f, btnWidth, btnHeight}, BLUE, btnFont, "White CheckMask", displayCheckMaskWhite },
		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 9.5f, btnWidth, btnHeight}, BLUE, btnFont, "Black CheckMask", displayCheckMaskBlack },
		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 11, btnWidth, btnHeight}, BLUE, btnFont, "White PinMask", displayPinMaskWhite },
		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 12, btnWidth, btnHeight}, BLUE, btnFont, "Black PinMask", displayPinMaskBlack },
		new ToggleButton { Rectangle {buttonsX, offsetY + btnDist * 13.5f, btnWidth, btnHeight}, BLUE, btnFont, "Book Moves", displayBookMoves },
	};

	Input fenInput = Input(Rectangle{ offsetX, SquareSize * 8 + offsetY + 10, SquareSize * 8, 30}, SKYBLUE, 20, "FEN");

private:
	// Core
	void updateData();
	void makeMoveToSelectedSquare();
	int calculateSelectedSquare(Vector2 mousePosition);
	std::vector<Chess::Move> getMovesFromSquare(int square);
	void loadBookMoves();

	void makeMove(Chess::Move move);
	void unmakeMove();

	// Sound Effects
	Sound getMoveSound(Chess::Move move);

	// Texture Loading
	Texture2D toCroppedTexture(int col, int row);
	void loadTextures();

	// Drawing
	void drawSearchData();
	void drawTime();
	void drawGameOver();
	void drawEvaluation();
	Color getCellColor(int col, int row);

	// window
	void resize();

public:
	Game();
	~Game();

	bool isRunning();
	void update();
	void draw();
};