#include "ChessBoard.h"
#include <bit>


namespace Chess
{
	bool ChessBoard::isSlidingPiece(uint64_t piece) const
	{
		return piece &
			(bitboards[Piece::WhiteRook] | bitboards[Piece::WhiteBishop] | bitboards[Piece::WhiteQueen] |
				bitboards[Piece::BlackRook] | bitboards[Piece::BlackBishop] | bitboards[Piece::BlackQueen]);
	}

	inline bool ChessBoard::isDiagonalSlider(uint64_t piece) const
	{
		return piece & (bitboards[Piece::WhiteBishop] | bitboards[Piece::BlackBishop] | bitboards[Piece::WhiteQueen] | bitboards[Piece::BlackQueen]);
	}

	inline bool ChessBoard::isVerticalSlider(uint64_t piece) const
	{
		return piece & (bitboards[Piece::WhiteRook] | bitboards[Piece::BlackRook] | bitboards[Piece::WhiteQueen] | bitboards[Piece::BlackQueen]);
	}


	uint64_t ChessBoard::computeCheckMask(bool white, uint64_t king) const
	{
		int colorMask = white ? Piece::White : Piece::Black;
		int oppositeMask = white ? Piece::Black : Piece::White;

		uint64_t checkingPieces = 0;

		// same color mask for pawns as if we generate moves from white king we want to check pawns left top and right top (similar rules apply to black)
		uint64_t pawnAttacks = getThreatMapforPawn(bitboards[Piece::King | colorMask], white);
		uint64_t knightAttacks = getThreatMapforKnight(bitboards[Piece::King | colorMask]);
		// Pass !white (DO NOT EXCLUDE ENEMY KING: getOccupiedSquares() & ~enemyKing;)
		uint64_t bihsopAttacks = getThreatMapforDiagonal(bitboards[Piece::King | colorMask], !white);
		uint64_t rookAttacks = getThreatMapforVertical(bitboards[Piece::King | colorMask], !white);
		uint64_t queenAttacks = getThreatMapforQueen(bitboards[Piece::King | colorMask], !white);

		checkingPieces |= bitboards[Piece::Pawn | oppositeMask] & pawnAttacks;
		checkingPieces |= bitboards[Piece::Knight | oppositeMask] & knightAttacks;
		checkingPieces |= bitboards[Piece::Bishop | oppositeMask] & bihsopAttacks;
		checkingPieces |= bitboards[Piece::Rook | oppositeMask] & rookAttacks;
		checkingPieces |= bitboards[Piece::Queen | oppositeMask] & queenAttacks;


		// return 0 if double check means only king moves are valid
		if (std::popcount(checkingPieces) > 1)
		{
			return 0;
		}

		// Only 1 checking piece
		if (!isSlidingPiece(checkingPieces))
		{
			return checkingPieces;
		}

		// only 1 checking sliding piece
		return checkingPieces | raysBetween[std::countr_zero(checkingPieces)][std::countr_zero(king)];
	}

	inline uint64_t ChessBoard::computeD12PinMask(bool white) const
	{
		static const int directions[4] = { -7, 7, -9, 9 };

		uint64_t king = bitboards[white ? Piece::WhiteKing : Piece::BlackKing];
		if (king == 0)
		{
			king = 0;
		}
		uint64_t occupiedByAlly = getOccupiedSquares(white);
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		uint64_t pinMask = 0ULL;

		for (const auto& dir : directions)
		{
			uint64_t curPos = king;
			bool metFriendlyPiece = false;

			while (true)
			{
				if ((dir == 7) && (curPos & (COL1 | ROW8))) break;		// BOTTOM-LEFT
				if ((dir == -7) && (curPos & (COL8 | ROW1))) break;		// TOP-RIGHT
				if ((dir == 9) && (curPos & (COL8 | ROW8))) break;		// BOTTOM-RIGHT
				if ((dir == -9) && (curPos & (COL1 | ROW1))) break;		// TOP-LEFT

				curPos = dir > 0 ? curPos << dir : curPos >> -dir;

				if (curPos == 0) break;

				if (occupiedByAlly & curPos)
				{
					if (metFriendlyPiece) break;
					metFriendlyPiece = true;
				}

				if (occupiedByEnemy & curPos)
				{
					if (metFriendlyPiece && isDiagonalSlider(curPos))
					{
						pinMask |= raysBetween[std::countr_zero(king)][std::countr_zero(curPos)];
						pinMask |= curPos;
					}
					break;
				}
			}
		}

		return pinMask;
	}

	inline uint64_t ChessBoard::computeHVPinMask(bool white) const
	{
		static const int directions[8] = { -1, 1, -8, 8 };

		uint64_t king = bitboards[white ? Piece::WhiteKing : Piece::BlackKing];
		if (king == 0)
		{
			king = 0;
		}
		uint64_t occupiedByAlly = getOccupiedSquares(white);
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		uint64_t pinMask = 0ULL;

		for (const auto& dir : directions)
		{
			uint64_t curPos = king;
			bool metFriendlyPiece = false;

			while (true)
			{
				if ((dir == 1) && (curPos & COL8)) break;				// RIGHT
				if ((dir == -1) && (curPos & COL1)) break;				// LEFT
				if ((dir == 8) && (curPos & ROW8)) break;				// BOTTOM
				if ((dir == -8) && (curPos & ROW1)) break;				// TOP

				curPos = dir > 0 ? curPos << dir : curPos >> -dir;

				if (curPos == 0) break;

				if (occupiedByAlly & curPos)
				{
					if (metFriendlyPiece) break;
					metFriendlyPiece = true;
				}

				if (occupiedByEnemy & curPos)
				{
					if (metFriendlyPiece && isVerticalSlider(curPos))
					{
						pinMask |= raysBetween[std::countr_zero(king)][std::countr_zero(curPos)];
						pinMask |= curPos;
					}
					break;
				}
			}
		}

		return pinMask;
	}


	uint64_t ChessBoard::getPinMask(bool white) const
	{
		return (computeHVPinMask(white) | computeD12PinMask(white));
	}

	uint64_t ChessBoard::getCheckMask(bool white) const
	{
		return computeCheckMask(white, bitboards[white ? Piece::WhiteKing : Piece::BlackKing]);
	}
}