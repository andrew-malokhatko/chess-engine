#pragma once
#include <string>
#include <unordered_map>

#include "Move.h"

namespace Chess
{
	class Book
	{
		// store zobristKey[move1:count, move2:count, move3:count...]
		const int maxPlyCount = 10;
		std::unordered_map<uint64_t, std::unordered_map<Move, int>> openingBook = {};

	public:
		// Initialize Book
		static Book loadFromPGN(const std::string& fileName, int maxPlyCount);
		static Book loadFromBook(const std::string& fileName, int maxPlyCount);

		// Get move from Book
		Move getFirstBookMove(uint64_t zobristKey) const;
		Move getRandomBookMove(uint64_t zobristKey) const;
		Move getWeightedBookMove(uint64_t zobristKey) const;

		std::unordered_map<Move, int> getAllBookMoves(uint64_t zobristKey) const;

		// Used in BookParser
		void insert(uint64_t zobristKey, Move move);
		void insertMap(uint64_t zobristKey, std::unordered_map<Move, int>);

		const std::unordered_map<uint64_t, std::unordered_map<Move, int>> getBook() const { return openingBook;  }
	};
}