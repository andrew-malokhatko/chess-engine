#pragma once

#include "AI.h"
#include "ChessBoard.h"
#include "Debug.h"

#include <array>
#include <vector>
#include <chrono>
#include <thread>

namespace Chess
{
	struct GameData
	{
		std::array<Chess::Piece, 64> board;
		std::vector<Chess::Move> moves;
		Chess::GameState gameState;
		std::chrono::milliseconds timeWhite;
		std::chrono::milliseconds timeBlack;
		bool isKingInCheck;
	};

	enum class GameMode
	{
		Computer,
		Human,
	};

	enum class TimeControl
	{
		Rapid = 600000,
		Blitz = 180000,
		Bullet = 60000,
	};

	class GameManager
	{
	private:
		bool playerToMove = true;
		bool computerWhite = false;
		bool discardSearchResult = false;

		bool calculating = false;
		bool gameInProgress = false;

		float positionEvaluation = 0.0f;

		ChessBoard chessBoard;
		AI computer;
		GameMode gameMode;
		TimeControl timeControl;
	
		std::chrono::milliseconds timeWhite;
		std::chrono::milliseconds timeBlack;

		std::chrono::steady_clock::time_point lastUpdate;

		std::string currentFen = std::string(Consts::startFen);

		std::jthread computerCalculations;

	private:
		void makeMove(Move move);
		void computerMakeMove();

	public:
		GameManager(void);
		GameManager(GameMode gameMode, TimeControl timeControl);

		// Core
		void startNewGame();
		void runGame();
		void update();
		void processMove(Move move);
		void unmakeMove();


		// Getters
		GameData getGameData() const;
		DebugData getDebugData() const;
		float getEvaluation() const;

		// Setters
		void loadPositionFromFEN(const std::string FEN);
		void setGameMode(GameMode gameMode);
		void setComputerWhite(bool white);
		void setTimeControl(TimeControl timeControl);

		
		// Tests
		void runTests() const;
	};
}