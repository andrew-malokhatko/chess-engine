#include <cassert>
#include <chrono>

#include "Book.h"
#include "BookParser.h"

namespace Chess
{

	Book Book::loadFromPGN(const std::string& fileName, int maxPlyCount)
	{
		BookParser::MaxPlyCount = maxPlyCount;
		return BookParser::movesFromPGN(fileName);
	}


	Book Book::loadFromBook(const std::string& fileName, int maxPlyCount)
	{
		BookParser::MaxPlyCount = maxPlyCount;
		return BookParser::movesFromBook(fileName);
	}

	Move Book::getFirstBookMove(uint64_t zobristKey) const
	{
		if (!openingBook.contains(zobristKey))
		{
			return Move{ 0, 0 };
		}

		return (*openingBook.at(zobristKey).begin()).first;
	}

	Move Book::getRandomBookMove(uint64_t zobristKey) const
	{
		if (!openingBook.contains(zobristKey))
		{
			return Move{ 0, 0 };
		}

		auto moveMap = openingBook.at(zobristKey);
		auto randomIt = std::next(std::begin(moveMap), rand() % moveMap.size());

		return (*randomIt).first;
	}

	Move Book::getWeightedBookMove(uint64_t zobristKey) const
	{
		if (!openingBook.contains(zobristKey))
		{
			return Move{ 0, 0 };
		}

		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		auto moveMap = openingBook.at(zobristKey);
		int totalMoves = 0;

		for (const auto& move : moveMap)
		{
			totalMoves += move.second;
		}

		float random = ((float)rand() / RAND_MAX);
		float curMoveCount = 0.0f;

		for (const auto& move : moveMap)
		{
			curMoveCount += (float)move.second;

			if ((curMoveCount / totalMoves) >= random)
			{
				return move.first;
			}
		}

		// have not found random move, somehting went wrong, return first element
		assert(0);
		return (*moveMap.begin()).first;
	}

	std::unordered_map<Move, int> Book::getAllBookMoves(uint64_t zobristKey) const
	{
		if (!openingBook.contains(zobristKey))
		{
			return std::unordered_map<Move, int>{};
		}

		return openingBook.at(zobristKey);
	}

	void Book::insert(uint64_t zobristKey, Move move)
	{
		if (openingBook.contains(zobristKey))
		{
			auto& moveMap = openingBook.at(zobristKey);

			if (moveMap.contains(move))
			{
				moveMap[move]++;
			}
			else
			{
				moveMap[move] = 1;
			}
		}
		else
		{
			std::unordered_map<Move, int> moveMap;
			moveMap[move] = 1;

			openingBook[zobristKey] = moveMap;
		}
	}

	void Book::insertMap(uint64_t zobristKey, std::unordered_map<Move, int> movesFromKey)
	{
		openingBook[zobristKey] = movesFromKey;
	}
}