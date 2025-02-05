#pragma once

#include <cstdint>
#include <array>
#include "ChessBoard.h"

namespace Chess
{
	class Zobrist
	{
	private:
		static constexpr int NumEnPassantFiles = 9;
		static constexpr int NumCastlingRights = 16;
		static constexpr uint64_t seed = 123456;

	public:
		// starting position, FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
		static constexpr uint64_t startingPosition = 15346820377121847993;

		// Set of seeded random numbers are generated for each piece, its color and its position
		static std::array<std::array<uint64_t, 64>, Consts::TotalBitboards> piecesArray;

		// Together 4 castling rights 1111 which result int 16 possible values
		static std::array<uint64_t, 16> castlingRights;

		// 8 enPassant file + 1(index 0) no enPassant
		static std::array<uint64_t, 9> enPassantFiles;
		static uint64_t sideToMove;

		Zobrist();
		static void initZobristKeys();
		static uint64_t calculateZobristKey(const ChessBoard& chessBoard);
	};
}