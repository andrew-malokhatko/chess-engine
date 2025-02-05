#include <bit>

#include "ChessBoard.h"

namespace Chess
{
	void ChessBoard::generatePawnMoves(uint64_t pawns, bool white, bool genQuiets)
	{
		uint64_t occupiedSquares = getOccupiedSquares();
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		while (pawns)
		{
			uint64_t moves = 0;
			uint64_t doublePush = 0;
			uint64_t curPawn = 1ULL << std::countr_zero(pawns);
			uint64_t enPassantRank = white ? ROW4 : ROW5;

			// Check if pawn is pinned HV and D12 separately as
			// pawn can move through 2 pins so we need to chech how manually
 			bool isPinnedHV = masks.pinHV & curPawn;
			bool isPinnedD12 = masks.pinD12 & curPawn;

			// generate pinMask based on type of pin
			uint64_t curPinMask = isPinnedHV ? masks.pinHV : isPinnedD12 ? masks.pinD12 : 0xffffffffffffffff;

			Move::Flag flag = Move::None;

			if (white)
			{
				moves |= (curPawn >> 7) & ~COL1 & occupiedByEnemy;								// Capture Right
				moves |= (curPawn >> 9) & ~COL8 & occupiedByEnemy;								// Capture Left
				if (curPawn & ROW2) flag = Move::PromotionQueen;								// Add promotion flag

				if (genQuiets)
				{
					moves |= (curPawn >> 8) & ~occupiedSquares;												// Single Push
					doublePush = ((((curPawn & ROW7) >> 8) & ~occupiedSquares) >> 8) & (~occupiedSquares);	// Calculate double push
				}
			}
			else
			{
				moves |= (curPawn << 9) & ~COL1 & occupiedByEnemy;										// Capture Right
				moves |= (curPawn << 7) & ~COL8 & occupiedByEnemy;										// Capture Left
				if (curPawn & ROW7) flag = Move::PromotionQueen;										// Add promotion flag

				if (genQuiets)
				{
					moves |= (curPawn << 8) & ~occupiedSquares;												// Single Push
					doublePush = ((((curPawn & ROW2) << 8) & ~occupiedSquares) << 8) & (~occupiedSquares);	// Calculate double push
				}
			}

			// Add double push move
			if (doublePush & curPinMask & masks.checkMask)
			{
				legalMoves[lastMoveIndex] = Move{ std::countr_zero(curPawn), std::countr_zero(doublePush), Move::DoublePush };
				lastMoveIndex++;
			}

			// Check for enPassant moves
			if ((enPassantRank & curPawn) && gameState.hasEnPassant())
			{
				uint64_t enPassantSquare = 1ULL << gameState.getEnPassantSquare() & curPinMask;

				// If generated for black, capture square is 1 row below enPassant square
				// If generated for white, capture square is 1 row above enPassant square
				uint64_t enPassantCapture = white ? (enPassantSquare << 8) : (enPassantSquare >> 8);
				bool isNearEnPassant = (((curPawn & ~COL8) << 1) | ((curPawn & ~COL1) >> 1)) & enPassantCapture;

				// Compare enPassant capture with current check mask
				enPassantCapture &= masks.checkMask;

				// Check if the pawn is near enPassant square, additionally if capture is valid (enPassantSquare and capture exist)
				if (enPassantSquare && enPassantCapture && isNearEnPassant)
				{
					int colorMask = white ? Piece::White : Piece::Black;
					int oppositeMask = white ? Piece::Black : Piece::White;

					// Check for enPassant capture (2 pawns leave king in check on enpassant row case)
					// remove 2 pawns from bitboards (enPassant and currect)
					// do not forget to add new pawn position FEN: (8/2p5/3p4/KP4k1/5p1r/8/4P1P1/6R1 w - - 0 1)
					bitboards[Piece::Pawn | oppositeMask] &= ~enPassantCapture;
					bitboards[Piece::Pawn | colorMask] &= ~curPawn;
					bitboards[Piece::Pawn | colorMask] |= enPassantSquare;

					// Check if king is in check after enPassant capture
					uint64_t threatMap = getThreatMap(!white);

					// Place pawns back after calculating threatMap and remove enPassant capture
					bitboards[Piece::Pawn | oppositeMask] |= enPassantCapture;
					bitboards[Piece::Pawn | colorMask] |= curPawn;
					bitboards[Piece::Pawn | colorMask] &= ~enPassantSquare;

					if (isKingInCheck(white, threatMap) == false)
					{
						legalMoves[lastMoveIndex] = Move{ std::countr_zero(curPawn) , std::countr_zero(enPassantSquare), Move::EnPassant };
						lastMoveIndex++;
					}
				}
			}

			moves &= curPinMask;
			moves &= masks.checkMask;

			while (moves)
			{
				if (flag == Move::PromotionQueen)
				{
					legalMoves[lastMoveIndex] = Move{ std::countr_zero(curPawn), std::countr_zero(moves), Move::PromotionQueen };
					lastMoveIndex++;
					/*for (int i = Move::PromotionRook; i <= Move::PromotionQueen; i++)
					{
						legalMoves[lastMoveIndex] = Move{ std::countr_zero(curPawn), std::countr_zero(moves), i };
						lastMoveIndex++;
					}*/
				}
				else
				{
					legalMoves[lastMoveIndex] = Move{ std::countr_zero(curPawn), std::countr_zero(moves) };
					lastMoveIndex++;
				}

				//allMoves.push_back(Move{ std::countr_zero(pawns) , std::countr_zero(moves), flag });
				moves &= (moves - 1);
			}

			pawns &= (pawns - 1);
		}
	}

	void ChessBoard::generateKingMoves(uint64_t king, bool white, bool genQuiets)
	{
		uint64_t occupiedSquares = getOccupiedSquares();
		uint64_t occupiedByAlly = getOccupiedSquares(white);
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		uint64_t castlingKingsideMask = white ? WhiteCastlingKingsideMask : BlackCastlingKingsideMask;
		uint64_t castlingQueensideMask = white ? WhiteCastlingQueensideMask : BlackCastlingQueensideMask;

		uint64_t moves = 0;

		moves |= (king & ~COL8) << 1;	// RIGHT
		moves |= (king & ~COL1) >> 1;	// LEFT
		moves |= (king & ~ROW8) << 8;	// DOWN
		moves |= (king & ~ROW1) >> 8;	// TOP

		moves |= (king & ~ROW8 & ~COL8) << 9;	// RIGHT-DOWN
		moves |= (king & ~ROW1 & ~COL1) >> 9;	// LEFT-TOP
		moves |= (king & ~ROW8 & ~COL1) << 7;	// LEFT-DOWN
		moves |= (king & ~ROW1 & ~COL8) >> 7;	// RIGHT-TOP

		moves &= ~occupiedByAlly;
		moves &= ~masks.threatMap;

		if (!genQuiets)
		{
			moves &= occupiedByEnemy;
		}

		// Generate castling moves
		bool kingInCheck = king & masks.threatMap;
		bool canCastleKingside = (castlingKingsideMask & (occupiedSquares | masks.threatMap)) == 0;

		// Pop less significant bit in queenside mask to check if path is safe
		bool queenSidePathEmpty = (castlingQueensideMask & occupiedSquares) == 0;
		bool queenSidePathSafe = ((castlingQueensideMask - 1) & castlingQueensideMask & masks.threatMap) == 0;
		bool canCastleQueenside = queenSidePathEmpty && queenSidePathSafe;

		if (gameState.getKingsideCastlingRights(white) && canCastleKingside && !kingInCheck)
		{
			legalMoves[lastMoveIndex] = Move{ std::countr_zero(king) , std::countr_zero(king << 2), Move::CastlingKingside };
			lastMoveIndex++;
		}

		if (gameState.getQueensideCastlingRights(white) && canCastleQueenside && !kingInCheck)
		{
			legalMoves[lastMoveIndex] = Move{ std::countr_zero(king) , std::countr_zero(king >> 2), Move::CastlingQueenside };
			lastMoveIndex++;
		}

		while (moves)
		{
			legalMoves[lastMoveIndex] = Move{ std::countr_zero(king) , std::countr_zero(moves) };
			lastMoveIndex++;
			moves &= (moves - 1);
		}
	}

	void ChessBoard::generateKnightMoves(uint64_t knights, bool white, bool genQuiets)
	{
		uint64_t occupiedByAlly = getOccupiedSquares(white);
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		while (knights)
		{
			uint64_t moves = 0;
			uint64_t curKnight = 1ULL << std::countr_zero(knights);

			moves |= (curKnight & ~ROW7 & ~ROW8 & ~COL8) << 17;		// DOWN-RIGHT
			moves |= (curKnight & ~ROW7 & ~ROW8 & ~COL1) << 15;		// DOWN-LEFT
			moves |= (curKnight & ~ROW1 & ~ROW2 & ~COL1) >> 17;		// TOP-LEFT
			moves |= (curKnight & ~ROW1 & ~ROW2 & ~COL8) >> 15;		// TOP-RIGHT

			moves |= (curKnight & ~COL7 & ~COL8 & ~ROW8) << 10;		// RIGHT-DOWN
			moves |= (curKnight & ~COL7 & ~COL8 & ~ROW1) >> 6;		// RIGHT-TOP
			moves |= (curKnight & ~COL1 & ~COL2 & ~ROW8) << 6;		// LEFT-DOWN
			moves |= (curKnight & ~COL1 & ~COL2 & ~ROW1) >> 10;		// LEFT-TOP

			moves &= ~occupiedByAlly;
			moves &= masks.checkMask;

			if (!genQuiets)
			{
				moves &= occupiedByEnemy;
			}

			// Check if knight is pinned
			if ((masks.pinHV | masks.pinD12) & curKnight)
			{
				// Knight cannot move if pinned
				moves = 0ULL;
			}

			while (moves)
			{
				legalMoves[lastMoveIndex] = Move{ std::countr_zero(curKnight), std::countr_zero(moves) };
				lastMoveIndex++;
				moves &= (moves - 1);
			}

			knights &= (knights - 1);
		}
	}

	void ChessBoard::generateSlidingDiagonalMoves(uint64_t pieces, bool white, bool genQuiets)
	{
		static constexpr int directions[4] = { 7, -7, 9, -9 };

		uint64_t occupiedByAlly = getOccupiedSquares(white);
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		uint64_t moves = 0;

		while (pieces)
		{
			uint64_t curPiece = 1ULL << std::countr_zero(pieces);

			// Remove all moves if diagonal slider is pinned horizontally (r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/5q2/P2P1RPP/q2Q1K2 w kq - 0 1)
			uint64_t curPinHV = (masks.pinHV & curPiece) ? 0x0 : 0xffffffffffffffff;
			uint64_t curPinD12 = (masks.pinD12 & curPiece) ? masks.pinD12 : 0xffffffffffffffff;

			for (const auto& dir : directions)
			{
				uint64_t curPos = curPiece;

				while (true)
				{
					if ((dir == 7) && (curPos & (COL1 | ROW8))) break;		// BOTTOM-LEFT
					if ((dir == -7) && (curPos & (COL8 | ROW1))) break;		// TOP-RIGHT
					if ((dir == 9) && (curPos & (COL8 | ROW8))) break;		// BOTTOM-RIGHT
					if ((dir == -9) && (curPos & (COL1 | ROW1))) break;		// TOP-LEFT

					curPos = dir > 0 ? curPos << dir : curPos >> -dir;

					if (occupiedByAlly & curPos) break;

					moves |= curPos;

					if (occupiedByEnemy & curPos) break;
				}
			}

			moves &= masks.checkMask;
			moves &= curPinHV;
			moves &= curPinD12;

			if (!genQuiets)
			{
				moves &= occupiedByEnemy;
			}

			while (moves)
			{
				legalMoves[lastMoveIndex] = Move{ std::countr_zero(pieces), std::countr_zero(moves) };
				lastMoveIndex++;
				moves &= (moves - 1);
			}

			pieces &= (pieces - 1);
		}
	}

	void ChessBoard::generateSlidingVerticalMoves(uint64_t pieces, bool white, bool genQuiets)
	{
		static constexpr int directions[4] = { 1, -1, 8, -8 };

		uint64_t occupiedByAlly = getOccupiedSquares(white);
		uint64_t occupiedByEnemy = getOccupiedSquares(!white);

		uint64_t moves = 0;

		while (pieces)
		{
			uint64_t curPiece = 1ULL << std::countr_zero(pieces);

			// remove all moves if vertical slider is pinned diagonally (r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/5q2/P2P1RPP/q2Q1K2 w kq - 0 1)
			uint64_t curPinHV = (masks.pinHV & curPiece) ? masks.pinHV : 0xffffffffffffffff;
			uint64_t curPinD12 = (masks.pinD12 & curPiece) ? 0x0 : 0xffffffffffffffff;

			for (const auto& dir : directions)
			{
				uint64_t curPos = curPiece;

				while (true)
				{
					if ((dir == 1) && (curPos & COL8)) break;	// RIGHT
					if ((dir == -1) && (curPos & COL1)) break;	// LEFT
					if ((dir == 8) && (curPos & ROW8)) break;	// BOTTOM
					if ((dir == -8) && (curPos & ROW1)) break;	// TOP

					curPos = dir > 0 ? curPos << dir : curPos >> -dir;

					if (occupiedByAlly & curPos) break;

					moves |= curPos;

					if (occupiedByEnemy & curPos) break;
				}
			}

			moves &= masks.checkMask;
			moves &= curPinD12;
			moves &= curPinHV;

			if (!genQuiets)
			{
				moves &= occupiedByEnemy;
			}

			while (moves)
			{
				legalMoves[lastMoveIndex] = Move{ std::countr_zero(pieces), std::countr_zero(moves) };
				lastMoveIndex++;
				moves &= (moves - 1);
			}

			pieces &= (pieces - 1);
		}
	}
}