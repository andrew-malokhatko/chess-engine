#include <cassert>
#include <bit>

#include "Evaluation.h"
#include "PieceSquareTables.h"

namespace Chess::Evaluation
{
	int flipFull(int sq)
	{
		assert(sq >= 0 && sq < 64);
		return (sq ^ 56);
	}

	// Add points for each piece, prioritize queens and rooks
	float calculateGamePhase(const ChessBoard& chessBoard)
	{
		// 0 to 100 here
		int gamePhase = 0;

		std::array<uint64_t, 15> bitboards = chessBoard.getBitboards();
		GameState gameState = chessBoard.getGameState();

		// add up all pieces
		gamePhase += std::popcount(bitboards[Piece::WhiteQueen] | bitboards[Piece::BlackQueen]) * 10;
		gamePhase += std::popcount(bitboards[Piece::WhiteRook] | bitboards[Piece::BlackRook]) * 5;
		gamePhase += std::popcount(bitboards[Piece::WhiteKnight] | bitboards[Piece::BlackKnight]) * 3;
		gamePhase += std::popcount(bitboards[Piece::WhiteBishop] | bitboards[Piece::BlackBishop]) * 4;
		gamePhase += std::popcount(bitboards[Piece::WhitePawn] | bitboards[Piece::BlackPawn]) / 2;

		gamePhase += 5 * (int)gameState.getKingsideCastlingRights(true);
		gamePhase += 5 * (int)gameState.getQueensideCastlingRights(true);
		gamePhase += 5 * (int)gameState.getKingsideCastlingRights(false);
		gamePhase += 5 * (int)gameState.getQueensideCastlingRights(false);

		assert(gamePhase <= 105);

		return (float)gamePhase / 100.0f;
	}

	int applyPSTWhite(uint64_t pieces, const int* PST)
	{
		int evaluation = 0;

		while (pieces)
		{
			evaluation += PST[std::countr_zero(pieces)];
			pieces &= (pieces - 1);
		}

		return evaluation;
	}


	int applyPSTBlack(uint64_t pieces, const int* PST)
	{
		int evaluation = 0;

		while (pieces)
		{
			evaluation += PST[flipFull(std::countr_zero(pieces))];
			pieces &= (pieces - 1);
		}

		return evaluation;
	}

	int applyPassedPawns(uint64_t pawns, uint64_t enemyPawns, bool white)
	{
		int colorIndex = int(white);
		int bonus = 0;

		uint64_t promotionRow = white ? Consts::ROW2 : Consts::ROW7;

		bonus += std::popcount(promotionRow & pawns) * passedPawnBonus;
		pawns &= ~promotionRow;

		while (pawns)
		{
			int square = std::countr_zero(pawns);
			uint64_t passedPawnMask = passedPawnTables[colorIndex][square];
			
			if ((passedPawnMask & enemyPawns) == 0ULL)
			{
				bonus += passedPawnBonus;
			}

			pawns &= (pawns - 1);
		}

		return bonus;
	}

	int applyIsolatedPawns(uint64_t pawns)
	{
		int penalty = 0;

		// create pawns full as we change pawns when iterating
		uint64_t pawnsFull = pawns;

		while (pawns)
		{
			int pawnIndex = std::countr_zero(pawns);
			int col = pawnIndex % 8;

			if ((isolatedPawnTable[col] & pawnsFull) == 0ULL)
			{
				penalty += isolatedPawnPenalty;
			}

			pawns &= (pawns - 1);
		}
		
		return penalty;
	}

	int applyDefenderPawns(uint64_t pawns, uint64_t ocupiedSquares, bool white)
	{
		int evaluation = 0;
		uint64_t defendedSquares = ChessBoard::getThreatMapforPawn(pawns, white);
		
		evaluation += std::popcount(ocupiedSquares & defendedSquares) * defenderBonus;

		return evaluation;
	}
	
	int applyConnectedPawns(uint64_t pawns)
	{
		uint64_t pawnsFull = pawns;
		pawns &= ~Consts::COL8;
	
		// shift pawns 1 to the right and check if there is another pawn
		// therefore bonus for 2 connected pawns in 20, 3 - 40, 4 - 60....
		return std::popcount((pawns << 1) & pawnsFull) * connectedPawnsBonus;
	}

	int applyAttackedSquares(uint64_t threatMap)
	{
		const uint64_t center = 0b00000000'00000000'00111100'00111100'00111100'00111100'00000000'00000000;
		const int attackedCenterSquareBonus = 3;

		int evaluation = 0;

		evaluation += std::popcount(threatMap & ~center) * attackedSquaresBonus;
		evaluation += std::popcount(threatMap & center) * attackedCenterSquareBonus;

		return evaluation;
	}

	int applyKnightBonuses(uint64_t knights, uint64_t friendlyPawns, uint64_t enemyPawns, bool white)
	{
		const int knightOutPostBonus = 40;
		const int lotPawnsBonus = 3;
		const int defendedByPawnBonus = 20;

		const int underDevelopedPenalty = 25;
		const int pawnsForPenalty = 14;

		int colorIndex = int(white);
		int evaluation = 0;

		uint64_t defendedSquares = ChessBoard::getThreatMapforPawn(friendlyPawns, white);

		evaluation -= (pawnsForPenalty - ((std::popcount(friendlyPawns | enemyPawns)) * std::popcount(knights))) * lotPawnsBonus;
		evaluation += std::popcount(knights & defendedSquares) * defendedByPawnBonus;

		evaluation -= std::popcount(knights & (Consts::ROW1 | Consts::ROW8)) * underDevelopedPenalty;

		while (knights)
		{
			// Check for the outpost using passedPawnTable
			if ((passedPawnTables[colorIndex][std::countr_zero(knights)] & enemyPawns) == 0ULL)
			{
				evaluation += knightOutPostBonus;
			}
			knights &= (knights - 1);
		}

		return evaluation;
	}

	int applyBishopBonuses(uint64_t bishops)
	{
		const int bishopPairBonus = 70;
		const int underDevelopedPenalty = 25;

		int evaluation = 0;

		if (std::popcount(bishops) > 1) evaluation += bishopPairBonus;

		evaluation -= std::popcount(bishops & (Consts::ROW1 | Consts::ROW8)) * underDevelopedPenalty;

		return evaluation;
	}

	int applyRookBonuses(uint64_t rooks, uint64_t occupiedSquares)
	{
		const int rookOnOpenFileBonus = 39;
		const int defendingEachOtherBonus = 50;
		int evaluation = 0;

		occupiedSquares &= ~rooks;

		uint64_t anyRookIndex = std::countr_zero(rooks);
		uint64_t rookFile = anyRookIndex % 8;
		uint64_t rookRank = anyRookIndex / 8;

		if (std::popcount(rookRank & rooks) > 1 || std::popcount(rookFile & rooks) > 1)
		{
			evaluation += defendingEachOtherBonus;
		}

		while (rooks)
		{
			int col = std::countr_zero(rooks) % 8;

			if ((filesTable[col] & occupiedSquares) == 0ULL)
			{
				evaluation += rookOnOpenFileBonus;
			}
			rooks &= (rooks - 1);
		}

		return evaluation;
	}

	int applyOccupiedCenter(uint64_t occupiedSquares)
	{
		const int CenterPieceBonus = 8;
		const uint64_t center = 0b00000000'00000000'00111100'00111100'00111100'00111100'00000000'00000000;

		return std::popcount(center & occupiedSquares) * CenterPieceBonus;
	}

	// specific for endgames
	int forceKingToCorner(uint64_t king, uint64_t opponentKing)
	{
		int evaluation = 0;

		int friendlyIndex = std::countr_zero(king);
		int opponentIndex = std::countr_zero(opponentKing);

		int opponentRank = opponentIndex / 8;
		int opponentFile = opponentIndex % 8;

		// force opponent king to the corner
		int opponentDistanceCenterFile = std::max(3 - opponentFile, opponentFile - 4);
		int opponentDistanceCenterRank = std::max(3 - opponentRank, opponentRank - 4);
		int opponentDistanceFromCenter = opponentDistanceCenterFile + opponentDistanceCenterRank;
		evaluation += opponentDistanceFromCenter;

		// distance between kings
		int friendlyRank = friendlyIndex / 8;
		int friendlyFile = friendlyIndex % 8;

		int distanceRank = std::abs(friendlyRank - opponentRank);
		int distanceFile = std::abs(friendlyFile - opponentFile);
		int distanceKings = distanceFile + distanceRank;

		evaluation += 15 - (distanceFile + distanceRank);

		return evaluation * 10;
	}

	int EvaluatePositionStatic(const ChessBoard& chessBoard)
	{
		GameState boardState = chessBoard.getGameState();
		bool gameOver = boardState.isGameOver();

		uint64_t occupiedSquaresWhite = chessBoard.getOccupiedSquares(true);
		uint64_t occupiedSquaresBlack = chessBoard.getOccupiedSquares(false);
		uint64_t occupiedSquares = occupiedSquaresBlack | occupiedSquaresWhite;

		if (gameOver)
		{
			if (boardState.whiteWon())
			{
				return PosInfinity;
			}
			else if (boardState.blackWon())
			{
				return NegInfinity;
			}

			return 0;
		}

		int evaluation = 0;
		std::array<uint64_t, 15> bitboards = chessBoard.getBitboards();

		// float from 0 to 1, where 1 is start and 0 is end
		float gamePhase = calculateGamePhase(chessBoard);

		int startWhite = Piece::WhiteRook;
		int endWhite = Piece::WhitePawn;

		int startBlack = Piece::BlackRook;
		int endBlack = Piece::BlackPawn;

		for (int i = startWhite; i <= endWhite; i++)
		{
			evaluation += std::popcount(bitboards[i]) * pieceValues[i];

			evaluation += gamePhase * (float)applyPSTWhite(bitboards[i], Evaluation::mg_pestoTable[i]);
			evaluation += (1.0f - gamePhase) * (float)applyPSTWhite(bitboards[i], Evaluation::eg_pestoTable[i]);
		}

		for (int i = startBlack; i <= endBlack; i++)
		{
			evaluation -= std::popcount(bitboards[i]) * pieceValues[i - Piece::Black];

			evaluation -= gamePhase * (float)applyPSTBlack(bitboards[i], Evaluation::mg_pestoTable[i - Piece::Black]);
			evaluation -= (1.0f - gamePhase) * (float)applyPSTBlack(bitboards[i], Evaluation::eg_pestoTable[i - Piece::Black]);
		}

		// apply penalties for doubled pawns
		evaluation -= std::popcount(bitboards[Piece::WhitePawn] & (bitboards[Piece::WhitePawn] >> 8)) * doubledPawnPenalty;
		evaluation += std::popcount(bitboards[Piece::BlackPawn] & (bitboards[Piece::BlackPawn] << 8)) * doubledPawnPenalty;

		// apply bonuses for passed pawns
		evaluation += (1.0f - gamePhase) * applyPassedPawns(bitboards[Piece::WhitePawn], bitboards[Piece::BlackPawn], true);
		evaluation -= (1.0f - gamePhase) * applyPassedPawns(bitboards[Piece::BlackPawn], bitboards[Piece::WhitePawn], false);

		// apply penalties for isolated pawns 
		evaluation -= gamePhase * applyIsolatedPawns(bitboards[Piece::WhitePawn]);
		evaluation += gamePhase * applyIsolatedPawns(bitboards[Piece::BlackPawn]);

		// apply bonuses for every piece defended by a pawn
		evaluation += gamePhase * applyDefenderPawns(bitboards[Piece::WhitePawn], occupiedSquaresWhite, true);
		evaluation -= gamePhase * applyDefenderPawns(bitboards[Piece::BlackPawn], occupiedSquaresBlack, false);

		// apply bonuses for connected pawns
		evaluation += gamePhase * applyConnectedPawns(bitboards[Piece::WhitePawn]);
		evaluation -= gamePhase * applyConnectedPawns(bitboards[Piece::BlackPawn]);

		// bonuses for attacked squares
		evaluation += gamePhase * applyAttackedSquares(chessBoard.getThreatMap(true));
		evaluation -= gamePhase * applyAttackedSquares(chessBoard.getThreatMap(false));

		// apply knight bonuses and penalties (outpost, low pawn number)
		evaluation += applyKnightBonuses(bitboards[Piece::WhiteKnight], bitboards[Piece::WhitePawn], bitboards[Piece::BlackPawn], true);
		evaluation -= applyKnightBonuses(bitboards[Piece::BlackKnight], bitboards[Piece::BlackPawn], bitboards[Piece::WhitePawn], false);

		// apply bishop bonuses
		evaluation += applyBishopBonuses(bitboards[Piece::WhiteBishop]);
		evaluation -= applyBishopBonuses(bitboards[Piece::BlackBishop]);

		// apply rook bonuses
		evaluation += applyRookBonuses(bitboards[Piece::WhiteRook], occupiedSquares);
		evaluation -= applyRookBonuses(bitboards[Piece::BlackRook], occupiedSquares);

		// force opponent king to corner
		int kingToCenterBonus = (1.0f - gamePhase) * forceKingToCorner(bitboards[Piece::WhiteKing], bitboards[Piece::BlackKing]);
		
		evaluation += chessBoard.isWhiteToMove() ? kingToCenterBonus : -kingToCenterBonus;

		evaluation += gamePhase * applyOccupiedCenter(occupiedSquaresWhite);
		evaluation -= gamePhase * applyOccupiedCenter(occupiedSquaresBlack);

		return evaluation;
	}

	int Evaluation::EvaluatePosition(const ChessBoard& chessBoard)
	{
		return chessBoard.isWhiteToMove() ?
			EvaluatePositionStatic(chessBoard) :
			-EvaluatePositionStatic(chessBoard);
	}
}