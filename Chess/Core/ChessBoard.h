#pragma once

#include <vector>
#include <array>
#include <string>

#include "Pieces.h"
#include "Move.h"
#include "GameState.h"
#include "ChessBoardConsts.h"
#include "Masks.h"



namespace Chess
{
	class ChessBoard : private Consts
	{
		// Position information
		bool whiteToMove = true;
		GameState gameState;

		std::array<uint64_t, TotalBitboards> bitboards;

		// Move generation
		// Store moves in array instead of vector for speed
		std::array<Move, MaxPossibleMoves> legalMoves;
		size_t lastMoveIndex = 0;
		Masks masks;

		// Making/unmaking a move
		uint64_t zobristKey = 0ULL;

		// Stack pointer serves the role to track indices of pastStates, bitboards etc
		int stackPointer = -1;
		std::array<std::array<uint64_t, TotalBitboards>, stackSize> pastPositions;
		std::array<GameState, stackSize> pastGameStates;
		std::array<uint64_t, stackSize> pastZobristKeys;

	public:
		ChessBoard();
		~ChessBoard();

	private:
		void loadStartingPosition();
	
	public:
		void reset();

		// Getters
		std::array<Piece, 64> getBoardAsArray() const;
		std::vector<Move> getLegalMovesAsVector() const;	// not optimized for performance!!

		std::array<Move, MaxPossibleMoves> getLegalMoves() { return legalMoves; }
		bool isWhiteToMove() const { return whiteToMove; }
		GameState getGameState() const { return gameState; }
		GameState& getGameStateRef() { return gameState; }
		size_t getMovesSize() const { return lastMoveIndex; }
		uint64_t getZobristKey() const { return zobristKey; }
		Piece getPiece(uint64_t square) const;
		Piece getPieceType(int index) const;
		//Piece getPieceType(uint64_t piecePos) const;

		// Move generation
		size_t generateMovesToDepth(int depth);
		void generateMoves(bool genQuiets = true);

		// Masks & threatMaps
		uint64_t computeCheckMask(bool white, uint64_t kingMask) const;
		uint64_t computeD12PinMask(bool white) const;
		uint64_t computeHVPinMask(bool white) const;
		uint64_t getCheckMask(bool white) const;
		uint64_t getPinMask(bool white) const;
		uint64_t getThreatMap(bool white) const;
		// returns threatmap for opponent, checkmask and pinmask for self
		const Masks& getMasks() const { return masks; };

		// Make and Unmake Move
		void makeMove(const Move& move);
		// pops last move from gameState
		void unmakeMove();
		bool canUnmakeMove() const { return stackPointer >= 0; };
			
		// Make unmake special moves
		void handlePromotionMove(uint64_t pawn, Piece newPiece, Piece pawnColor);
		void handleEnPassantMove(uint64_t pawn, Piece oppositePawnColor, bool white);
		void handleCastlingMove(uint64_t king, Piece friendlyRook, Move::Flag castlingType);

		void rollbackCastlingMove(uint64_t king, Piece friendlyRook, Move::Flag castlingType);
		void updateCastlingRights(uint64_t fromMask, uint64_t toMask, bool white);

		// Utility methods
		uint64_t getOccupiedSquares(bool white) const;
		uint64_t getOccupiedSquares() const;
		bool isSlidingPiece(uint64_t piece) const;
		bool isDiagonalSlider(uint64_t piece) const;
		bool isVerticalSlider(uint64_t piece) const;
		bool isKingInCheck(bool white) const;
		bool isKingInCheck(bool white, uint64_t threatMap) const;

		// Loading and debugging
		void loadPosFromFen(const std::string& position, bool updateZobristKey = true);
		int perft(int depth);
		void perftTest(int depth);

		// Chess notation and coords
		static std::string toChessNotation(Move move);
		static std::string indexToCoord(int index);

		// Move generation
		void generateKingMoves(uint64_t king, bool white, bool genQuiets);
		void generatePawnMoves(uint64_t pawns, bool white, bool genQuiets);
		void generateKnightMoves(uint64_t knights, bool white, bool genQuiets);
		void generateSlidingDiagonalMoves(uint64_t piece, bool white, bool genQuiets);
		void generateSlidingVerticalMoves(uint64_t piece, bool white, bool genQuiets);

		// ThreatMaps
		static uint64_t getThreatMapforPawn(uint64_t pawns, bool white);
		static uint64_t getThreatMapforKing(uint64_t king);
		static uint64_t getThreatMapforKnight(uint64_t knights);
		uint64_t getThreatMapforDiagonal(uint64_t pieces, bool white) const;
		uint64_t getThreatMapforVertical(uint64_t pieces, bool white) const;
		uint64_t getThreatMapforQueen(uint64_t queens, bool white) const;

		// Copying
		ChessBoard shallowCopy();	// copies only current gameState and position, no pastPositions and gameStates
		std::array<uint64_t, TotalBitboards> getBitboards() const;
	};
}