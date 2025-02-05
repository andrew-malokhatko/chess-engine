#pragma once

#include "ChessBoard.h"
#include <cassert>


namespace Chess::Evaluation
{
	// black table on index 0, white ttable on index 1
	consteval std::array<std::array<uint64_t, 64>, 2> precomputePassedPawnTables()
	{
		std::array<uint64_t, 64> passedPawnTableWhite{ };
		std::array<uint64_t, 64> passedPawnTableBlack{ };

		std::array<std::array<uint64_t, 64>, 2> passedPawnTable;

		// start from 16(row3) end at 56(row7) WHITE
		for (int i = 16; i < 56; i++)
		{
			int col = i % 8;
			int row = i / 8;

			uint64_t pawnRowMask = 1ULL << i;
			uint64_t LeftRowMask = col != 0 ? (1ULL << i) >> 1 : 0ULL;
			uint64_t RightRowMask = col != 7 ? (1ULL << i) << 1 : 0ULL;

			uint64_t fullPawnRowMask = pawnRowMask | LeftRowMask | RightRowMask;
			uint64_t resultMask = 0ULL;

			assert(row < 7 && row >= 2);

			int k = 1;
			while ((row - k) > 0)
			{
				resultMask |= fullPawnRowMask >> (8 * k);
				k++;
			}

			passedPawnTableWhite[i] = resultMask;
		}

		// start from 8(row2) end at 48(row6) BLACK

		for (int i = 8; i < 48; i++)
		{
			int col = i % 8;
			int row = i / 8;

			uint64_t pawnRowMask = 1ULL << i;
			uint64_t LeftRowMask = col != 0 ? (1ULL << i) >> 1 : 0ULL;
			uint64_t RightRowMask = col != 7 ? (1ULL << i) << 1 : 0ULL;

			uint64_t fullPawnRowMask = pawnRowMask | LeftRowMask | RightRowMask;
			uint64_t resultMask = 0ULL;

			assert(row < 6 && row >= 1);

			int k = 1;
			while ((row + k) < 8)
			{
				resultMask |= fullPawnRowMask << (8 * k);
				k++;
			}

			passedPawnTableBlack[i] = resultMask;
		}

		passedPawnTable[0] = passedPawnTableBlack;
		passedPawnTable[1] = passedPawnTableWhite;

		return passedPawnTable;
	}

	consteval std::array<uint64_t, 8> precomputeIsolatedPawnTables()
	{
		std::array<uint64_t, 8> isolatedPawnTable{};

		for (int col = 0; col < 8; col++)
		{
			uint64_t LeftRowMask = col != 0 ? (1ULL << col) >> 1 : 0ULL;
			uint64_t RightRowMask = col != 7 ? (1ULL << col) << 1 : 0ULL;

			uint64_t neighborRows = LeftRowMask | RightRowMask;
			uint64_t resultMask = 0ULL;

			for (int row = 0; row < 8; row++)
			{
				resultMask |= neighborRows << (8 * row);
			}

			isolatedPawnTable[col] = resultMask;
		}

		return isolatedPawnTable;
	}

	consteval std::array<uint64_t, 8> precomputeFilesTable()
	{
		std::array<uint64_t, 8> files{};

		for (int col = 0; col < 8; col++)
		{
			uint64_t file = 1ULL << col;

			for (int row = 0; row < 8; row++)
			{
				file |= file << (8 * row);
			}
			files[col] = file;
		}

		return files;
	}

	// Precomputed tables to identify passed pawns ([0] for black [1] for white)
	static constexpr std::array<std::array<uint64_t, 64>, 2> passedPawnTables = precomputePassedPawnTables();
	static constexpr std::array<uint64_t, 8> isolatedPawnTable = precomputeIsolatedPawnTables();
	static constexpr std::array<uint64_t, 8> filesTable = precomputeFilesTable();

	static constexpr int PosInfinity = INT32_MAX - 100;
	static constexpr int NegInfinity = INT32_MIN + 100;

	static constexpr int passedPawnBonus = 110;
	static constexpr int isolatedPawnPenalty = 35;
	static constexpr int doubledPawnPenalty = 30;
	static constexpr int defenderBonus = 11;
	static constexpr int connectedPawnsBonus = 6;
	static constexpr int attackedSquaresBonus = 2;

	// Matches the indices of Pieces.h
	static constexpr std::array<int, 7> pieceValues = {
		0,			// None
		500,		// Rook
		320,		// Knight
		350,		// Bishop
		900,		// Queen
		10000,		// King
		105			// Pawn
	};

	float calculateGamePhase(const ChessBoard& chessBoard);

	// Static (signed evaluation)
	int EvaluatePositionStatic(const ChessBoard& chessBoard);

	// Evaluation based on player to move
	int EvaluatePosition(const ChessBoard& chessBoard);
}