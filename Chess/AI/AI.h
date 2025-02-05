#pragma once

#include <unordered_map>
#include <chrono>

#include "ChessBoard.h"
#include "Debug.h"

#include "Book.h"
#include "Search.h"

#if !defined(LICHESS_BOOK_PATH) || !defined(MASTER_BOOK_PATH)

#define MASTER_BOOK_PATH "./Books/MasterBook.txt"
#define LICHESS_BOOK_PATH "./Books/LichessBook.txt"

#endif

namespace Chess
{
	class AI
	{
		static constexpr const char* masterBookName = MASTER_BOOK_PATH;
		static constexpr const char* lichessBookName = LICHESS_BOOK_PATH;
		static constexpr int maxPlyCount = 10;

		const Book masterBook = Book::loadFromBook(masterBookName, maxPlyCount);
		const Book lichessBook = Book::loadFromBook(lichessBookName, maxPlyCount);

		Search search;

		static constexpr std::chrono::milliseconds delayBookMillis = std::chrono::milliseconds(250);

		std::chrono::milliseconds timeLeft = std::chrono::milliseconds(0);
		std::chrono::milliseconds searchTime = std::chrono::milliseconds(0);

		bool isFollowingBook = true;
		bool isSearching = false;
		int evaluation = 0;

	public:
		bool white = false;

		// Move generation
		Move getRandomMove(ChessBoard chessBoard);
		Move getBestMove(ChessBoard board);
		Move getBookMove(uint64_t zobristKey) const;

		void forceStopSearch();
		void updateTime(std::chrono::milliseconds timeSinceLastUpdate);
		void setTimeLeft(std::chrono::milliseconds timeLeft);

		std::unordered_map<Move, int> getAllBookMoves(const ChessBoard& board) const;
		SearchStats getSearchStats() const { return search.getSearchStats(); };
		int getEvaluation() const { return evaluation; };

		void reset();
	};
}