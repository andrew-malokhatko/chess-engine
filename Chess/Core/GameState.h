#pragma once

#include <cstdint>
#include "Move.h"

namespace Chess
{
	class GameState
	{
		static constexpr uint16_t CASTLING_RIGHTS = 0b1111;
		static constexpr uint16_t WHITE_KINGSIDE_CASTLING = 1 << 0;
		static constexpr uint16_t WHITE_QUEENSIDE_CASTLING = 1 << 1;
		static constexpr uint16_t BLACK_KINGSIDE_CASTLING = 1 << 2;
		static constexpr uint16_t BLACK_QUEENSIDE_CASTLING = 1 << 3;
		static constexpr uint16_t WHITE_WON = 1 << 4;
		static constexpr uint16_t BLACK_WON = 1 << 5;
		static constexpr uint16_t STALEMATE = 1 << 6;

		// En passant mask and offsets
		static constexpr uint16_t EN_PASSANT_FILE_MASK = 0b111 << 7;		// Bits 7–9
		static constexpr uint16_t EN_PASSANT_RANK_MASK = 0b111 << 10;		// Bits 10–12
		static constexpr uint16_t EN_PASSANT_NO_SQUARE = 0b111111 << 7;		// Special value for no en passant

		// Lastly Captured piece
		static constexpr uint16_t CAPTURED_PIECE = 0b111 << 13;				// bits 13 - 15

		// last Move (encoded)
		static constexpr uint32_t MOVE_MASK = 0b11111111'11111111 << 16;

		// Allow all castling rights in the beginning + disable en passant
		uint32_t state = CASTLING_RIGHTS | EN_PASSANT_NO_SQUARE;

	public:
		// Used for zobrist hashing
		uint16_t getCastlingRights() const
		{
			return state & CASTLING_RIGHTS;
		}

		bool getKingsideCastlingRights(bool white) const
		{
			return state & (white ? WHITE_KINGSIDE_CASTLING : BLACK_KINGSIDE_CASTLING);
		}

		bool getQueensideCastlingRights(bool white) const
		{
			return state & (white ? WHITE_QUEENSIDE_CASTLING : BLACK_QUEENSIDE_CASTLING);
		}

		void setKingsideCastlingRights(bool white, bool rights)
		{
			if (!rights) state &= white ? ~(WHITE_KINGSIDE_CASTLING) : ~(BLACK_KINGSIDE_CASTLING);
			else state |= white ? WHITE_KINGSIDE_CASTLING : BLACK_KINGSIDE_CASTLING;
		}

		void setQueensideCastlingRights(bool white, bool rights)
		{
			if (!rights) state &= white ? ~(WHITE_QUEENSIDE_CASTLING) : ~(BLACK_QUEENSIDE_CASTLING);
			else state |= white ? WHITE_QUEENSIDE_CASTLING : BLACK_QUEENSIDE_CASTLING;
		}

		bool isGameOver() const
		{
			return state & (WHITE_WON | BLACK_WON | STALEMATE);
		}

		void setWinner(bool white)
		{
			state |= white ? WHITE_WON : BLACK_WON;
		}

		bool whiteWon() const
		{
			return state & WHITE_WON;
		}

		bool blackWon() const
		{
			return state & BLACK_WON;
		}

		bool isStalemate() const
		{
			return state & STALEMATE;
		}

		void setStalemate()
		{
			state |= STALEMATE;
		}

		// En passant
		int getEnPassantSquare() const
		{
			uint16_t encoded = (state & (EN_PASSANT_FILE_MASK | EN_PASSANT_RANK_MASK)) >> 7;
			if (encoded == EN_PASSANT_NO_SQUARE) return -1;
			int file = encoded & 0b111;
			int rank = (encoded >> 3) & 0b111;
			return rank * 8 + file;
		}

		// Files from 1 to 8, no enPassant = 0
		int getEnPassantFile() const
		{
			int square = getEnPassantSquare();
			return square == -1 ? 0 : (square % 8) + 1;
		}

		// Ranks from 1 to 8, no enPassant = 0
		int getEnPassantRank() const
		{
			int square = getEnPassantSquare();
			return square == -1 ? 0 : (square / 8) + 1;
		}

		void setEnPassantSquare(int square)
		{
			if (square >= 0 && square < 64)
			{
				int file = square % 8;
				int rank = square / 8;
				uint16_t encoded = (rank << 3) | file;
				state = (state & ~(EN_PASSANT_FILE_MASK | EN_PASSANT_RANK_MASK)) | (encoded << 7);
			}
			else
			{
				state |= (EN_PASSANT_NO_SQUARE); // Set to "no en passant"
			}
		}

		void clearEnPassantSquare()
		{
			state |= (EN_PASSANT_NO_SQUARE); // Set to "no en passant"
		}

		bool hasEnPassant() const
		{
			return (state & (EN_PASSANT_FILE_MASK | EN_PASSANT_RANK_MASK)) != EN_PASSANT_NO_SQUARE;
		}

		void clearCapture()
		{
			state &= ~CAPTURED_PIECE;
		}

		void setCapture(int capturedPiece)
		{
			state &= ~CAPTURED_PIECE;
			state |= CAPTURED_PIECE & (capturedPiece << 13);
		}

		uint16_t getCapture() const
		{
			return (state & CAPTURED_PIECE) >> 13;
		}

		bool isCapture() const
		{
			return CAPTURED_PIECE & state;
		}

		void setLastMove(const Move& move)
		{
			uint32_t encodedMove = move.encode();

			// clear previous move
			state &= ~MOVE_MASK;

			state |= (encodedMove << 16);
		}

		Move getLastMove()
		{
			uint32_t encodedMove = (state & MOVE_MASK) >> 16;
			Move move = Move::decode(encodedMove);

			return move;
		}
	};
}