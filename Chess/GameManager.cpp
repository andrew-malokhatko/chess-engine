#include "GameManager.h"
#include "Zobrist.h"
#include "Tests.h"


namespace Chess
{
	GameManager::GameManager(GameMode gameMode, TimeControl timeControl) :
		gameMode {gameMode},
		timeControl{timeControl},
		lastUpdate{ std::chrono::high_resolution_clock::now()},
		timeWhite{ std::chrono::milliseconds(static_cast<int>(timeControl))},
		timeBlack{ std::chrono::milliseconds(static_cast<int>(timeControl))}
	{
		computer.setTimeLeft(std::chrono::milliseconds(static_cast<int>(timeControl)));
		Zobrist();
	}

	GameManager::GameManager(void) :
		gameMode {static_cast<int>(GameMode::Human)},
		timeControl{static_cast<int>(TimeControl::Blitz)},
		lastUpdate{ std::chrono::high_resolution_clock::now()},
		timeWhite{ std::chrono::milliseconds(static_cast<int>(timeControl)) },
		timeBlack{ std::chrono::milliseconds(static_cast<int>(timeControl)) }
	{
		computer.setTimeLeft(std::chrono::milliseconds(static_cast<int>(timeControl)));
		Zobrist();
	}

	void GameManager::update()
	{
		if (chessBoard.getGameState().isGameOver() || !gameInProgress) return;

		// make computer move is not player to move and we are not currently calculating
		if (gameMode == GameMode::Computer && !playerToMove && !calculating)
		{
			calculating = true;
			computerCalculations = std::jthread(&GameManager::computerMakeMove, this);
		}

		std::chrono::time_point curTime = std::chrono::high_resolution_clock::now();
		auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - lastUpdate);
		lastUpdate = curTime;

		if (chessBoard.isWhiteToMove() == computerWhite)
		{
			computer.updateTime(timeSinceLastUpdate);
		}

		if (chessBoard.isWhiteToMove())
		{
			timeWhite -= timeSinceLastUpdate;
			if (timeWhite.count() < 0)
			{
				chessBoard.getGameStateRef().setWinner(false);
				gameInProgress = false;
			}
		}
		else
		{
			timeBlack -= timeSinceLastUpdate;
			if (timeBlack.count() < 0)
			{
				chessBoard.getGameStateRef().setWinner(true);
				gameInProgress = false;
			}
		}
	}

	void GameManager::computerMakeMove()
	{
		//copy board and pass it to the computer
		ChessBoard boardCopy = chessBoard.shallowCopy();
		Move response = computer.getBestMove(boardCopy);

		if (!discardSearchResult)
		{
			makeMove(response);
			playerToMove = true;
		}

		positionEvaluation = (float)computer.getEvaluation() / 100;

		// set calculating to false when finished
		// + do not discard search result on the next calculation
		discardSearchResult = false;
		calculating = false;
	} 

	void GameManager::processMove(Move move)
	{
		// Ignore playerToMove, if human plays
		if (gameMode == GameMode::Human || playerToMove)
		{
			makeMove(move);
			playerToMove = false;
		}
	}

	GameData GameManager::getGameData() const
	{
		bool whiteToMove = chessBoard.isWhiteToMove();
		bool isKingInCheck = chessBoard.isKingInCheck(whiteToMove);

		return GameData {
			chessBoard.getBoardAsArray(),
			chessBoard.getLegalMovesAsVector(),
			chessBoard.getGameState(),
			timeWhite,
			timeBlack,
			isKingInCheck,
		};
	}

	DebugData GameManager::getDebugData() const
	{
		// Debug
		uint64_t threatMapWhite = chessBoard.getThreatMap(true);
		uint64_t pinMaskWhite = chessBoard.getPinMask(true);
		uint64_t checkMaskWhite = chessBoard.getCheckMask(true);

		uint64_t threatMapBlack = chessBoard.getThreatMap(false);
		uint64_t pinMaskBlack = chessBoard.getPinMask(false);
		uint64_t checkMaskBlack = chessBoard.getCheckMask(false);

		DebugMasks whiteMasks = DebugMasks{ threatMapWhite, pinMaskWhite, checkMaskWhite };
		DebugMasks blackMasks = DebugMasks{ threatMapBlack, pinMaskBlack, checkMaskBlack };

		const auto& bookMoves = computer.getAllBookMoves(chessBoard);

		SearchStats searchStats = computer.getSearchStats();

		return DebugData {
			whiteMasks,
			blackMasks,
			bookMoves,
			searchStats
		};
	}

	void GameManager::loadPositionFromFEN(const std::string FEN)
	{
		currentFen = FEN;

		startNewGame();
	}

	void GameManager::setComputerWhite(bool white)
	{
		computerWhite = white;
		playerToMove = !white;
	}

	// pass the color, so if gameMode is switched to computer the color is set to
	void GameManager::setGameMode(GameMode newGameMode)
	{ 
		gameMode = newGameMode;

		startNewGame();
	}

	void GameManager::setTimeControl(TimeControl newTimeControl)
	{
		timeControl = newTimeControl;
		computer.setTimeLeft(std::chrono::milliseconds(static_cast<int>(timeControl)));

		startNewGame();
	}

	void GameManager::startNewGame()
	{
		chessBoard.reset();
		computer.reset();

		if (currentFen != Consts::startFen)
		{
			chessBoard.loadPosFromFen(currentFen);
		}

		playerToMove = !computerWhite;
		gameInProgress = false;

		timeWhite = std::chrono::milliseconds(static_cast<int>(timeControl));
		timeBlack = std::chrono::milliseconds(static_cast<int>(timeControl));

		lastUpdate = std::chrono::high_resolution_clock::now();
	}

	void GameManager::runGame()
	{
		gameInProgress = true;

		lastUpdate = std::chrono::high_resolution_clock::now();
	}

	void GameManager::makeMove(Move move)
	{
		chessBoard.makeMove(move);
		chessBoard.generateMoves();		// get moves, because it automatically updates gameOver condition

		if (chessBoard.getGameState().isGameOver())
		{
			gameInProgress = false;
		}
	}

	void GameManager::unmakeMove()
	{
		if (!chessBoard.canUnmakeMove())
		{
			return;
		}

		chessBoard.unmakeMove();
		chessBoard.generateMoves();

		playerToMove = !playerToMove;

		if (calculating)
		{
			computer.forceStopSearch();
			discardSearchResult = true;
		}
	}

	void GameManager::runTests() const
	{
		Test::testMoveGeneration(Test::testGithub);
		Test::testMoveGeneration(Test::testDefault);
	}

	float GameManager::getEvaluation() const
	{
		return positionEvaluation;
	}
}