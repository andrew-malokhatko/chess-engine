#pragma once

#include <cstdint>
#include <string_view>

consteval uint64_t calculateRayBetween(int from, int to)
{
	int fromCol = from % 8;
	int fromRow = from / 8;
	int toCol = to % 8;
	int toRow = to / 8;

	int colDif = toCol - fromCol;
	int rowDif = toRow - fromRow;

	if (from == to) return 0ULL;

	int direction = 0;
	if (fromCol == toCol)
	{
		direction = rowDif > 0 ? 8 : -8;
	}
	else if (fromRow == toRow)
	{
		direction = colDif > 0 ? 1 : -1;
	}
	else if (colDif == rowDif)
	{
		direction = colDif > 0 ? 9 : -9;
	}
	else if (colDif == -rowDif)
	{
		direction = rowDif > 0 ? 7 : -7;
	}
	else
	{
		return 0;
	}

	uint64_t ray = 0ULL;
	int cur = from + direction;
	while (cur != to)
	{
		ray |= (1ULL << cur);
		cur += direction;
	}

	return ray;
}

consteval std::array<std::array<uint64_t, 64>, 64> precomputeRaysBetween()
{
	std::array<std::array<uint64_t, 64>, 64> allRays = {};

	// loop through every square in the board
	for (int row = 0; row < 8; row++)
	{
		for (int col = 0; col < 8; col++)
		{
			int startSquare = row * 8 + col;

			for (int r = 0; r < 8; r++)
			{
				for (int c = 0; c < 8; c++)
				{
					int endSquare = r * 8 + c;
					allRays[startSquare][endSquare] = calculateRayBetween(startSquare, endSquare);
				}
			}
		}
	}
	return allRays;
}


namespace Chess
{
	struct Consts
	{
		// total number is 12, but Piece indexing is (1, 2, 3, 4, 5, 6) | colorMask
		// this leads to 3 empty spots in the arrray of bitboards, so loops should be 
		// different for white and black pieces
		static constexpr int TotalBitboards = 15;
		static constexpr int UniquePieceNumber = 12;

		// stack size in plies (now game can last for maximum of 150 moves)
		static constexpr size_t stackSize = 1200;

		// Added 100 for safety (avoid memory reallocations at all cost)
		static constexpr int MaxPossibleMoves = 218 + 10;

		// precomputed rays for faster pin and check masks calculation
		static constexpr std::array<std::array<uint64_t, 64>, 64> raysBetween = precomputeRaysBetween();

		static constexpr std::string_view startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		static constexpr uint64_t COL1 = 0b0000000100000001000000010000000100000001000000010000000100000001;
		static constexpr uint64_t COL2 = 0b0000001000000010000000100000001000000010000000100000001000000010;
		static constexpr uint64_t COL7 = 0b0100000001000000010000000100000001000000010000000100000001000000;
		static constexpr uint64_t COL8 = 0b1000000010000000100000001000000010000000100000001000000010000000;

		static constexpr uint64_t ROW1 = 0b0000000000000000000000000000000000000000000000000000000011111111;
		static constexpr uint64_t ROW2 = 0b0000000000000000000000000000000000000000000000001111111100000000;
		static constexpr uint64_t ROW4 = 0b0000000000000000000000000000000011111111000000000000000000000000;
		static constexpr uint64_t ROW5 = 0b0000000000000000000000001111111100000000000000000000000000000000;
		static constexpr uint64_t ROW7 = 0b0000000011111111000000000000000000000000000000000000000000000000;
		static constexpr uint64_t ROW8 = 0b1111111100000000000000000000000000000000000000000000000000000000;

		// Castling Masks and Starting Positions
		static constexpr uint64_t BlackCastlingKingsideMask = 0b01100000;
		static constexpr uint64_t BlackCastlingQueensideMask = 0b00001110;

		static constexpr uint64_t BlackStartingKingSquare = 0b00010000;
		static constexpr uint64_t BlackStartingLeftRookSquare = 0b00000001;
		static constexpr uint64_t BlackStartingRightRookSquare = 0b10000000;

		static constexpr uint64_t WhiteCastlingKingsideMask = BlackCastlingKingsideMask << 56;
		static constexpr uint64_t WhiteCastlingQueensideMask = BlackCastlingQueensideMask << 56;

		static constexpr uint64_t WhiteStartingKingSquare = BlackStartingKingSquare << 56;
		static constexpr uint64_t WhiteStartingLeftRookSquare = BlackStartingLeftRookSquare << 56;
		static constexpr uint64_t WhiteStartingRightRookSquare = BlackStartingRightRookSquare << 56;
	};
}