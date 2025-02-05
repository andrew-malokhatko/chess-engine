#include "Zobrist.h"
#include <random>

namespace Chess
{
	// define all the arrays
	std::array<std::array<uint64_t, 64>, Consts::TotalBitboards> Zobrist::piecesArray { };
	std::array<uint64_t, 16> Zobrist::castlingRights { };
	std::array<uint64_t, 9> Zobrist::enPassantFiles { };
	uint64_t Zobrist::sideToMove { };

	Zobrist::Zobrist()
	{
		initZobristKeys();
	}

	void Zobrist::initZobristKeys()
	{
		std::mt19937_64 rng(seed);

		for (int square = 0; square < 64; square++)
		{
			// generate random values for white pieces and all possible positions
			for (int i = Piece::WhiteRook; i <= Piece::WhitePawn; i++)
			{
				piecesArray[i][square] = rng();
			}

			// generate randomg values for black pieces and all possible positions
			for (int i = Piece::BlackRook; i <= Piece::BlackPawn; i++)
			{
				piecesArray[i][square] = rng();
			}
		}

		for (int i = 0; i < NumCastlingRights; i++)
		{
			castlingRights[i] = rng();
		}

		// reserve first element indexed 0 for no EnPassant
		enPassantFiles[0] = 0;
		for (int i = 1; i < NumEnPassantFiles; i++)
		{
			enPassantFiles[i] = rng();
		}

		sideToMove = rng();
	}

	uint64_t Zobrist::calculateZobristKey(const ChessBoard& chessBoard)
	{
		uint64_t zobristKey = 0ULL;
		GameState boardState = chessBoard.getGameState();

		for (int squareIndex = 0; squareIndex < 64; squareIndex++)
		{
			uint64_t square = 1ULL << squareIndex;
			Piece piece = chessBoard.getPiece(square);

			zobristKey ^= piecesArray[piece][squareIndex];
		}

		int enPassantFile = boardState.getEnPassantFile();
		zobristKey ^= enPassantFiles[enPassantFile];

		uint16_t castlingMask = boardState.getCastlingRights();
		zobristKey ^= castlingRights[castlingMask];

		// apply sideToMove if it's black move
		if (!chessBoard.isWhiteToMove())
		{
			zobristKey ^= sideToMove;
		}

		return zobristKey;
	}
}