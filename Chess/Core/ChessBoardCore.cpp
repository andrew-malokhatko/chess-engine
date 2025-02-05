#include <bit>
#include <string>
#include <iostream>
#include <sstream>

#include "ChessBoard.h"
#include "Zobrist.h"

namespace Chess
{
	ChessBoard::ChessBoard()
	{
		// Can be used before Zobrist::initKeys(), uses constexpr Zobrist::StartingPositon
		loadStartingPosition();
		generateMoves();
	}

	ChessBoard::~ChessBoard()
	{
		return;
	}

	void ChessBoard::loadStartingPosition()
	{
		loadPosFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", false);

		// Use const startingPosition, as Zobrist Key calculation is quite slow
		zobristKey = Zobrist::startingPosition;
	}

	void ChessBoard::reset()
	{
		loadStartingPosition();

		std::cout << "-------------------------" << std::endl;

		// Clear past game information
		stackPointer = -1;

		gameState = GameState();
	}

	uint64_t ChessBoard::getOccupiedSquares() const
	{
		return	bitboards[Piece::WhitePawn] | bitboards[Piece::WhiteRook] | bitboards[Piece::WhiteKnight] |
			bitboards[Piece::WhiteBishop] | bitboards[Piece::WhiteQueen] | bitboards[Piece::WhiteKing] |
			bitboards[Piece::BlackPawn] | bitboards[Piece::BlackRook] | bitboards[Piece::BlackKnight] |
			bitboards[Piece::BlackBishop] | bitboards[Piece::BlackQueen] | bitboards[Piece::BlackKing];
	}

	uint64_t ChessBoard::getOccupiedSquares(bool white) const
	{
		return white ?
			bitboards[Piece::WhitePawn] | bitboards[Piece::WhiteRook] | bitboards[Piece::WhiteKnight] |
			bitboards[Piece::WhiteBishop] | bitboards[Piece::WhiteQueen] | bitboards[Piece::WhiteKing]
			:
			bitboards[Piece::BlackPawn] | bitboards[Piece::BlackRook] | bitboards[Piece::BlackKnight] |
			bitboards[Piece::BlackBishop] | bitboards[Piece::BlackQueen] | bitboards[Piece::BlackKing];
	}

	void ChessBoard::handlePromotionMove(uint64_t pawn, Piece newPiece, Piece pawnColor)
	{
		bitboards[pawnColor] &= ~pawn;
		bitboards[newPiece] |= pawn;
	}

	void ChessBoard::handleEnPassantMove(uint64_t pawn, Piece oppositePawnColor, bool white)
	{
		uint64_t square = white ? ~(pawn << 8) : ~(pawn >> 8);

		bitboards[oppositePawnColor] &= square;
		zobristKey ^= Zobrist::piecesArray[oppositePawnColor][std::countr_zero(square)];
	}

	void ChessBoard::handleCastlingMove(uint64_t king, Piece friendlyRook, Move::Flag castlingType)
	{
		if (castlingType == Move::CastlingKingside)
		{
			bitboards[friendlyRook] &= ~(king << 1);
			bitboards[friendlyRook] |= (king >> 1);

			zobristKey ^= Zobrist::piecesArray[friendlyRook][std::countr_zero(king << 1)];
			zobristKey ^= Zobrist::piecesArray[friendlyRook][std::countr_zero(king >> 1)];
		}
		else
		{
			bitboards[friendlyRook] &= ~(king >> 2);
			bitboards[friendlyRook] |= (king << 1);

			zobristKey ^= Zobrist::piecesArray[friendlyRook][std::countr_zero(king >> 2)];
			zobristKey ^= Zobrist::piecesArray[friendlyRook][std::countr_zero(king << 1)];
		}
	}

	void ChessBoard::updateCastlingRights(uint64_t fromMask, uint64_t toMask, bool white)
	{
		uint64_t kingSquare = white ? WhiteStartingKingSquare : BlackStartingKingSquare;
		uint64_t LeftRookSquare = white ? WhiteStartingLeftRookSquare : BlackStartingLeftRookSquare;
		uint64_t RightRookSquare = white ? WhiteStartingRightRookSquare : BlackStartingRightRookSquare;

		if ((fromMask | toMask) & kingSquare)
		{
			gameState.setKingsideCastlingRights(white, false);
			gameState.setQueensideCastlingRights(white, false);
		}
		else if ((fromMask | toMask) & LeftRookSquare)
		{
			gameState.setQueensideCastlingRights(white, false);
		}
		else if ((fromMask | toMask) & RightRookSquare)
		{
			gameState.setKingsideCastlingRights(white, false);
		}
	}

	void ChessBoard::makeMove(const Move& move)
	{
		//std::cout << "Zobrist key Before: " << zobristKey << std::endl;

		uint64_t fromMask = 1ULL << move.from;
		uint64_t toMask = 1ULL << move.to;

		// Store previous position before making move (used to unmake move)
		// Increment stack pointer before assigning values (starts from -1 )
		stackPointer++;

		pastPositions[stackPointer] = bitboards;
		pastGameStates[stackPointer] = gameState;
		pastZobristKeys[stackPointer] = zobristKey;

		int colorMask = whiteToMove ? Piece::White : Piece::Black;
		int oppositeColorMask = whiteToMove ? Piece::Black : Piece::White;

		int pieceFrom = Piece::Rook | colorMask;
		int pieceTo = Piece::Pawn | colorMask;

		for (int i = pieceFrom; i <= pieceTo; i++)
		{ 
			if (bitboards[i] & fromMask)
			{
				bitboards[i] &= ~fromMask;
				bitboards[i] |= toMask;

				zobristKey ^= Zobrist::piecesArray[i][move.from];	// remove piece from old position
				zobristKey ^= Zobrist::piecesArray[i][move.to];	// move piece to new position
				break;
			}
		}

		// Handle Captures
		pieceFrom = Piece::Rook | oppositeColorMask;
		pieceTo = Piece::Pawn | oppositeColorMask;

		// set new last move
		gameState.setLastMove(move);

		// Remove last capture, (unlike move should be cleared, because will not be overriden each move)
		gameState.clearCapture();

		for (int i = pieceFrom; i <= pieceTo; i++)
		{
			if (bitboards[i] & toMask)
			{
				bitboards[i] &= ~toMask;
				gameState.setCapture(i);
				zobristKey ^= Zobrist::piecesArray[i][move.to];
				break;
			}
		}

		// Construct correct PieceType using flag as Piece::Rook == Move::PromotionRook
		if (move.isPromotion())
		{
			// !!!!!! Always promote to queen qutomatically
			//handlePromotionMove(toMask, Piece{ Piece {move.flag}  | colorMask }, Piece{ Piece::Pawn | colorMask });
			handlePromotionMove(toMask, Piece{ Piece::Queen | colorMask }, Piece{ Piece::Pawn | colorMask });

			zobristKey ^= Zobrist::piecesArray[Piece::Pawn | colorMask][move.to];
			zobristKey ^= Zobrist::piecesArray[Piece::Queen | colorMask][move.to];
		}
		else if (move.isCastling())
		{
			// zobrist key updated inside of a funciton
			handleCastlingMove(toMask, Piece{ Piece::Rook | colorMask }, move.flag);
		}
		else if (move.flag == Move::Flag::EnPassant)
		{
			// zobrist key updated inside of a funciton
			handleEnPassantMove(toMask, Piece{ Piece::Pawn | oppositeColorMask }, whiteToMove);
			gameState.setCapture(Piece::Pawn);
		}

		if (move.flag == Move::Flag::DoublePush)
		{
			gameState.setEnPassantSquare(std::countr_zero(whiteToMove ? (toMask << 8) : (toMask >> 8)));
		}
		else
		{
			gameState.setEnPassantSquare(-1);
		}

		// update castling rights for both kings
		updateCastlingRights(fromMask, toMask, whiteToMove);
		updateCastlingRights(fromMask, toMask, !whiteToMove);

		// After new gameState is complete work on enPassant, castling rights and sideToMove inside zobristKey
		GameState prevGameState = pastGameStates[stackPointer];

		zobristKey ^= Zobrist::sideToMove;

		zobristKey ^= Zobrist::castlingRights[prevGameState.getCastlingRights()];
		zobristKey ^= Zobrist::castlingRights[gameState.getCastlingRights()];

		zobristKey ^= Zobrist::enPassantFiles[gameState.getEnPassantFile()];
		zobristKey ^= Zobrist::enPassantFiles[prevGameState.getEnPassantFile()];

		whiteToMove = !whiteToMove;
		
		//update gameOver conditions repetition
		int repetition = std::count(pastZobristKeys.begin(), pastZobristKeys.begin() + stackPointer, zobristKey);
		if (repetition >= 2)
		{
			std::cout << "Stalemate repetition";
			gameState.setStalemate();
		}

		// knight king, bishop king endgame
		uint64_t heavyPieces = bitboards[Piece::WhiteRook] | bitboards[Piece::BlackRook] | bitboards[Piece::WhiteQueen] | bitboards[Piece::BlackQueen];
		uint64_t pawns = bitboards[Piece::WhitePawn] | bitboards[Piece::BlackPawn];

		if (std::popcount(heavyPieces | pawns) == 0)
		{
			uint64_t lightPiecesWhite = bitboards[Piece::WhiteKnight] | bitboards[Piece::WhiteBishop];
			uint64_t lightPiecesBlack = bitboards[Piece::BlackKnight] | bitboards[Piece::BlackBishop];

			if (std::popcount(lightPiecesWhite) <= 1 && std::popcount(lightPiecesBlack) <= 1)
			{
				gameState.setStalemate();
				std::cout << "Stalemate Insufficient material";
			}
		}
	}

	void ChessBoard::rollbackCastlingMove(uint64_t king, Piece friendlyRook, Move::Flag castlingType)
	{
		if (castlingType == Move::CastlingKingside)
		{
			bitboards[friendlyRook] &= ~(king >> 1);
			bitboards[friendlyRook] |= (king << 1);
		}
		else
		{
			bitboards[friendlyRook] &= ~(king << 1);
			bitboards[friendlyRook] |= (king >> 2);
		}
	}

	void ChessBoard::unmakeMove()
	{
		// no previous moves? return
		if (stackPointer < 0)
		{
			return;
		}

		bitboards = pastPositions[stackPointer];
		gameState = pastGameStates[stackPointer];
		zobristKey = pastZobristKeys[stackPointer];
		stackPointer--;

		whiteToMove = !whiteToMove;

		// get move & last capture from current state
		/*Move move = gameState.getLastMove();

		uint64_t fromMask = 1ULL << move.from;
		uint64_t toMask = 1ULL << move.to;

		int colorMask = whiteToMove ? Piece::Black : Piece::White;
		int oppositeColorMask = whiteToMove ? Piece::White : Piece::Black;

		int pieceFrom = Piece::Rook | colorMask;
		int pieceTo = Piece::Pawn | colorMask;

		for (int i = pieceFrom; i <= pieceTo; i++)
		{
			if (bitboards[i] & toMask)
			{
				bitboards[i] &= ~toMask;
				bitboards[i] |= fromMask;
				break;
			}
		}

		// Restore captured piece if it was a capture
		if (gameState.isCapture())
		{
			int capturedPiece = (gameState.getCapture() | oppositeColorMask);
			bitboards[capturedPiece] |= toMask;
		}

		if (move.isPromotion())
		{
			// Seems cotrintuitive, but we have already moved queen back in the first loop
			// So now if move is promotion we just change queen to back pawn
			bitboards[Piece::Queen | colorMask] &= ~fromMask;
			bitboards[Piece::Pawn | colorMask] |= fromMask;
		}
		else if (move.isCastling())
		{
			rollbackCastlingMove(toMask, Piece{ Piece::Rook | colorMask }, move.flag);
		}
		else if (move.isEnPassant())
		{
			bitboards[Piece::Pawn | oppositeColorMask] &= ~toMask;
		}

		// Restore game state
		gameState = pastGameStates[stackPointer];
		zobristKey = pastZobristKeys[stackPointer];
		stackPointer--;

		// Restore turn
		whiteToMove = !whiteToMove;*/
	}


	// Starting a8 to h1 or horizontal left to right, top to bottom
	std::array<Piece, 64> ChessBoard::getBoardAsArray() const
	{
		std::array<Piece, 64> board;

		for (int i = 0; i < 64; i++)
		{
			uint64_t mask = 1ULL << i;

			if (bitboards[Piece::WhitePawn] & mask)				board[i] = Piece::WhitePawn;
			else if (bitboards[Piece::WhiteRook] & mask)		board[i] = Piece::WhiteRook;
			else if (bitboards[Piece::WhiteKnight] & mask)		board[i] = Piece::WhiteKnight;
			else if (bitboards[Piece::WhiteBishop] & mask)		board[i] = Piece::WhiteBishop;
			else if (bitboards[Piece::WhiteQueen] & mask)		board[i] = Piece::WhiteQueen;
			else if (bitboards[Piece::WhiteKing] & mask)		board[i] = Piece::WhiteKing;

			else if (bitboards[Piece::BlackPawn] & mask)		board[i] = Piece::BlackPawn;
			else if (bitboards[Piece::BlackRook] & mask)		board[i] = Piece::BlackRook;
			else if (bitboards[Piece::BlackKnight] & mask)		board[i] = Piece::BlackKnight;
			else if (bitboards[Piece::BlackBishop] & mask)		board[i] = Piece::BlackBishop;
			else if (bitboards[Piece::BlackQueen] & mask)		board[i] = Piece::BlackQueen;
			else if (bitboards[Piece::BlackKing] & mask)		board[i] = Piece::BlackKing;
		}

		return board;
	}

	std::vector<Move> ChessBoard::getLegalMovesAsVector() const
	{
		std::vector<Move> vectorLegalMoves{};
		vectorLegalMoves.reserve(MaxPossibleMoves);

		for (int i = 0; i < lastMoveIndex; i++)
		{
			vectorLegalMoves.emplace_back(legalMoves[i]);
		}

		return vectorLegalMoves;
	}

	size_t ChessBoard::generateMovesToDepth(int depth)
	{
		generateMoves();

		if (depth <= 1) return lastMoveIndex;

		size_t count = 0;

		for (const auto& move : legalMoves)
		{
			makeMove(move);
			count += generateMovesToDepth(depth - 1);
			unmakeMove();
		}

		return count;
	}

	void ChessBoard::perftTest(int depth)
	{
		generateMoves();
		int nodesSearched = 0;

		std::cout << "Depth: " << depth << std::endl;

		for (const auto& move : legalMoves)
		{
			makeMove(move);
			int moves = generateMovesToDepth(depth - 1);
			nodesSearched += moves;
			std::cout << toChessNotation(move) << " : " << moves << std::endl;
			unmakeMove();
		}

		std::cout << std::endl << "Nodes searched: " << nodesSearched << std::endl;
	}

	int ChessBoard::perft(int depth)
	{
		generateMoves();
		int nodesSearched = 0;

		if (depth < 1) return 0;
		if (depth == 1) return lastMoveIndex;

		for (const auto& move : legalMoves)
		{
			makeMove(move);
			int moves = generateMovesToDepth(depth - 1);
			nodesSearched += moves;
			unmakeMove();	
		}

		return nodesSearched;
	}

	void ChessBoard::generateMoves(bool genQuiets)
	{
		lastMoveIndex = 0;

		int colorMask = whiteToMove ? Piece::White : Piece::Black;
		
		masks = Masks{ getThreatMap(!whiteToMove), 0xffffffffffffffff, computeHVPinMask(whiteToMove), computeD12PinMask(whiteToMove) };

		bool kingInCheck = isKingInCheck(whiteToMove, masks.threatMap);

		// use existing threat map for isKingInCheck()
		// compute checkMask only if king is in check
		if (kingInCheck)
		{
			masks.checkMask = computeCheckMask(whiteToMove, bitboards[Piece::King | colorMask]);
		}

		generateKingMoves(bitboards[Piece::King | colorMask], whiteToMove, genQuiets);					// King
		generatePawnMoves(bitboards[Piece::Pawn | colorMask], whiteToMove, genQuiets);					// Pawns
		generateKnightMoves(bitboards[Piece::Knight | colorMask], whiteToMove, genQuiets);				// Knights
		generateSlidingDiagonalMoves(bitboards[Piece::Bishop | colorMask], whiteToMove, genQuiets);		// Bishops
		generateSlidingVerticalMoves(bitboards[Piece::Rook | colorMask], whiteToMove, genQuiets);		// Rooks
		generateSlidingDiagonalMoves(bitboards[Piece::Queen | colorMask], whiteToMove, genQuiets);		// Queens (diagonal and vertiacl move generation)
		generateSlidingVerticalMoves(bitboards[Piece::Queen | colorMask], whiteToMove, genQuiets);		// Queens

		if (lastMoveIndex == 0)
		{
			if (kingInCheck)
			{
				gameState.setWinner(!whiteToMove);
			}
			else
			{
				gameState.setStalemate();
			}
		}
	}

	// get square from and square to as a string
	std::string ChessBoard::toChessNotation(Move move)
	{
		std::string from = std::string(1, 'a' + (move.from % 8)) + std::to_string(8 - (move.from / 8));
		std::string to = std::string(1, 'a' + (move.to % 8)) + std::to_string(8 - (move.to / 8));
		return from + to;
	}

	std::string ChessBoard::indexToCoord(int index)
	{
		return std::string(1, 'a' + (index % 8)) + std::to_string(8 - (index / 8));
	}

	uint64_t ChessBoard::getThreatMap(bool white) const
	{
		return white ?
			getThreatMapforPawn(bitboards[Piece::WhitePawn], true) | getThreatMapforKing(bitboards[Piece::WhiteKing]) |
			getThreatMapforKnight(bitboards[Piece::WhiteKnight]) | getThreatMapforDiagonal(bitboards[Piece::WhiteBishop], true) |
			getThreatMapforVertical(bitboards[Piece::WhiteRook], true) | getThreatMapforQueen(bitboards[Piece::WhiteQueen], true)
			:
			getThreatMapforPawn(bitboards[Piece::BlackPawn], false) | getThreatMapforKing(bitboards[Piece::BlackKing]) |
			getThreatMapforKnight(bitboards[Piece::BlackKnight]) | getThreatMapforDiagonal(bitboards[Piece::BlackBishop], false) |
			getThreatMapforVertical(bitboards[Piece::BlackRook], false) | getThreatMapforQueen(bitboards[Piece::BlackQueen], false);
	}

	bool ChessBoard::isKingInCheck(bool white) const
	{
		return getThreatMap(!white) & (white ? bitboards[Piece::WhiteKing] : bitboards[Piece::BlackKing]);
	}

	bool ChessBoard::isKingInCheck(bool white, uint64_t threatMap) const
	{
		return threatMap & (white ? bitboards[Piece::WhiteKing] : bitboards[Piece::BlackKing]);
	}

	uint64_t ChessBoard::getThreatMapforPawn(uint64_t pawns, bool white)
	{
		uint64_t threatMap = 0;

		while (pawns)
		{
			uint64_t curPawn = 1ULL << std::countr_zero(pawns);

			if (white)
			{
				threatMap |= (curPawn >> 7) & ~COL1 & ~ROW8;	// Capture Right
				threatMap |= (curPawn >> 9) & ~COL8 & ~ROW8;	// Capture Left
			}
			else
			{
				threatMap |= (curPawn << 9) & ~COL1 & ~ROW1;	// Capture Right
				threatMap |= (curPawn << 7) & ~COL8 & ~ROW1;	// Capture Left
			}

			pawns &= (pawns - 1);
		}

		return threatMap;
	}

	uint64_t ChessBoard::getThreatMapforKing(uint64_t king)
	{
		uint64_t threatMap = 0;

		threatMap |= (king & ~COL8) << 1;	// RIGHT
		threatMap |= (king & ~COL1) >> 1;	// LEFT
		threatMap |= (king & ~ROW8) << 8;	// DOWN
		threatMap |= (king & ~ROW1) >> 8;	// TOP

		threatMap |= (king & ~ROW8 & ~COL8) << 9;	// RIGHT-DOWN
		threatMap |= (king & ~ROW1 & ~COL1) >> 9;	// LEFT-TOP
		threatMap |= (king & ~ROW8 & ~COL1) << 7;	// LEFT-DOWN
		threatMap |= (king & ~ROW1 & ~COL8) >> 7;	// RIGHT-TOP

		return threatMap;
	}


	uint64_t ChessBoard::getThreatMapforKnight(uint64_t knights)
	{
		uint64_t threatMap = 0;

		while (knights)
		{
			uint64_t curKnight = 1ULL << std::countr_zero(knights);

			threatMap |= (curKnight & ~ROW7 & ~ROW8 & ~COL8) << 17;		// DOWN-RIGHT
			threatMap |= (curKnight & ~ROW7 & ~ROW8 & ~COL1) << 15;		// DOWN-LEFT
			threatMap |= (curKnight & ~ROW1 & ~ROW2 & ~COL1) >> 17;		// TOP-LEFT
			threatMap |= (curKnight & ~ROW1 & ~ROW2 & ~COL8) >> 15;		// TOP-RIGHT

			threatMap |= (curKnight & ~COL7 & ~COL8 & ~ROW8) << 10;		// RIGHT-DOWN
			threatMap |= (curKnight & ~COL7 & ~COL8 & ~ROW1) >> 6;		// RIGHT-TOP
			threatMap |= (curKnight & ~COL1 & ~COL2 & ~ROW8) << 6;		// LEFT-DOWN
			threatMap |= (curKnight & ~COL1 & ~COL2 & ~ROW1) >> 10;		// LEFT-TOP

			knights &= (knights - 1);
		}

		return threatMap;
	}

	uint64_t ChessBoard::getThreatMapforDiagonal(uint64_t piece, bool white) const
	{
		static constexpr int directions[4] = { 7, -7, 9, -9 };

		// Exclude enemy king, so the goes through it.
		uint64_t enemyKing = bitboards[Piece::King | (white ? Piece::Black : Piece::White)];

		uint64_t occupiedSquares = getOccupiedSquares() & ~enemyKing;
		uint64_t threatMap = 0;

		while (piece)
		{
			uint64_t curPiece = 1ULL << std::countr_zero(piece);

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
					threatMap |= curPos;

					if (occupiedSquares & curPos) break;
				}
			}

			piece &= (piece - 1);
		}

		return threatMap;
	}
	uint64_t ChessBoard::getThreatMapforVertical(uint64_t piece, bool white) const
	{
		static constexpr int directions[4] = { 1, -1, 8, -8 };

		// Exclude enemy king, so the goes through it.
		uint64_t enemyKing = bitboards[Piece::King | (white ? Piece::Black : Piece::White)];

		uint64_t occupiedSquares = getOccupiedSquares() & ~enemyKing;
		uint64_t threatMap = 0;

		while (piece)
		{
			uint64_t curPiece = 1ULL << std::countr_zero(piece);

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
					threatMap |= curPos;

					if (occupiedSquares & curPos) break;
				}
			}

			piece &= (piece - 1);
		}

		return threatMap;
	}

	uint64_t ChessBoard::getThreatMapforQueen(uint64_t queens, bool white) const
	{
		return getThreatMapforDiagonal(queens, white) | getThreatMapforVertical(queens, white);
	}

	void ChessBoard::loadPosFromFen(const std::string& position, bool updateZobristKey)
	{
		// Reset board
		bitboards.fill(0);

		// Split FEN into tokens
		std::vector<std::string> tokens;
		std::basic_istringstream iss(position);
		std::string token;
		while (std::getline(iss, token, ' '))
		{
			tokens.push_back(token);
		}

		// Validate FEN tokens count
		if (tokens.size() < 4)
		{
			throw std::invalid_argument("Invalid FEN: insufficient tokens");
		}

		// Load pieces
		int rank = 0;
		int file = 0;
		for (const char& c : tokens[0])
		{
			if (c == '/')
			{
				rank++;
				file = 0;
			}
			else if (isdigit(c))
			{
				file += c - '0';
			}
			else
			{
				int piece = -1;
				switch (c)
				{
				case 'P': piece = Piece::WhitePawn; break;
				case 'R': piece = Piece::WhiteRook; break;
				case 'N': piece = Piece::WhiteKnight; break;
				case 'B': piece = Piece::WhiteBishop; break;
				case 'Q': piece = Piece::WhiteQueen; break;
				case 'K': piece = Piece::WhiteKing; break;
				case 'p': piece = Piece::BlackPawn; break;
				case 'r': piece = Piece::BlackRook; break;
				case 'n': piece = Piece::BlackKnight; break;
				case 'b': piece = Piece::BlackBishop; break;
				case 'q': piece = Piece::BlackQueen; break;
				case 'k': piece = Piece::BlackKing; break;
				}
				if (piece != -1)
				{
					bitboards[piece] |= 1ULL << (rank * 8 + file);
					file++;
				}
			}
		}

		// Load side to move
		whiteToMove = (tokens[1] == "w");

		// Load castling rights
		gameState.setKingsideCastlingRights(true, tokens[2].contains('K'));
		gameState.setQueensideCastlingRights(true, tokens[2].contains('Q'));
		gameState.setKingsideCastlingRights(false, tokens[2].contains('k'));
		gameState.setQueensideCastlingRights(false, tokens[2].contains('q'));

		// Load en passant square
		if (tokens[3] != "-")
		{
			int file = tokens[3][0] - 'a';
			int rank = 7 - (tokens[3][1] - '1');
			if (file >= 0 && file < 8 && rank >= 0 && rank < 8)
			{
				gameState.setEnPassantSquare(rank * 8 + file);
			}
			else
			{
				throw std::invalid_argument("Invalid FEN: en passant square out of bounds");
			}
		}
		else
		{
			gameState.setEnPassantSquare(-1);
		}

		// Calculate Zobrist key when new position is loaded
		if (updateZobristKey)
		{
			zobristKey = Zobrist::calculateZobristKey(*this);
		}

		// generate moves for newly updated position
		generateMoves();
	}

	ChessBoard ChessBoard::shallowCopy()
	{
		ChessBoard copy = ChessBoard();

		copy.bitboards = bitboards;
		copy.gameState = gameState;
		copy.whiteToMove = whiteToMove;
		copy.zobristKey = zobristKey;

		copy.legalMoves = legalMoves;
		copy.lastMoveIndex = lastMoveIndex;

		// set pastXonristKeys to detect move repetiotion
		copy.pastZobristKeys = pastZobristKeys;

		return copy;
	}

	std::array<uint64_t, ChessBoard::TotalBitboards> ChessBoard::getBitboards() const
	{
		return bitboards;
	}

	Piece ChessBoard::getPiece(uint64_t square) const
	{
		if ((getOccupiedSquares() & square) == 0) return Piece::None;

		for (int i = Piece::WhiteRook; i <= Piece::BlackPawn; i++)
		{
			if (square & bitboards[i])
			{
				return Piece{ i };
			}
		}

		return Piece::None;
	}

	Piece ChessBoard::getPieceType(int index) const
	{
		Piece piece = getPiece(1ULL << index);
		
		// remove color from piece
		return piece & ~(Piece::Black);
	}
}