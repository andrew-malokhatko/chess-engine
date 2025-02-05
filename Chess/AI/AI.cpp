#include "AI.h"
#include "Search.h"
#include <chrono>
#include <thread>

namespace Chess
{
	Move AI::getRandomMove(ChessBoard board)
	{
		std::array<Move, Consts::MaxPossibleMoves> legalMoves = board.getLegalMoves();

		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		if (!legalMoves.empty())
		{
			int randomIndex = std::rand() % legalMoves.size();
			return legalMoves[randomIndex];
		}
		else
		{
			return Move{ 0, 0 };
		}
	}

	std::unordered_map<Move, int> AI::getAllBookMoves(const ChessBoard& board) const
	{
		const auto& mainBook = masterBook.getAllBookMoves(board.getZobristKey());
		const auto& secondaryBook = lichessBook.getAllBookMoves(board.getZobristKey());

		if (mainBook.empty())
		{
			return secondaryBook;
		}

		return mainBook;
	}

	Move AI::getBookMove(uint64_t zobristKey) const
	{
		Move bookMove = masterBook.getWeightedBookMove(zobristKey);

		if (bookMove.isNullMove())
		{
			bookMove = lichessBook.getWeightedBookMove(zobristKey);
		}

		return bookMove;
	}
	void AI::setTimeLeft(std::chrono::milliseconds newTimeLeft)
	{
		timeLeft = newTimeLeft;
	}

	std::chrono::milliseconds calculateMaxSearchTime(std::chrono::milliseconds timeLeft)
	{
		return timeLeft / 300;
		//return std::chrono::milliseconds(10000000);
	}

	void AI::updateTime(std::chrono::milliseconds timeSinceLastUpdate)
	{
		timeLeft -= timeSinceLastUpdate;
		searchTime -= timeSinceLastUpdate;

		if (isSearching && (searchTime < std::chrono::milliseconds(0)))
		{
			search.stopSearch();
		}
	}

	void AI::forceStopSearch()
	{
		search.stopSearch();
	}

	Move AI::getBestMove(ChessBoard board)
	{
		if (isFollowingBook)
		{
			Move bookMove = getBookMove(board.getZobristKey());

			if (!bookMove.isNullMove())
			{
				// short delay to avoid instantaneous move being played
				std::this_thread::sleep_for(delayBookMillis);
				return bookMove;
			}
			else
			{
				isFollowingBook = false;
			}
		}

		searchTime = calculateMaxSearchTime(timeLeft);
		isSearching = true;

		Move bestMove = search.searchBestMove(board);
		evaluation = search.getEvaluation();

		isSearching = false;

		return bestMove;
	}

	void AI::reset()
	{
		isFollowingBook = true;
		search.clearHistory();
	};
}