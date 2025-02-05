
#include "Evaluation.h"
#include "Sort.h"
#include "Search.h"
#include <utility>
#include <iostream>

namespace Chess
{
	void Search::orderMoves(std::array<Move, Consts::MaxPossibleMoves>& moves, int moveSize)
	{
		std::array<int, Consts::MaxPossibleMoves> moveScores = {};
		bool whiteToMove = board.isWhiteToMove();


		for (int i = 0; i < moveSize; i++)
		{
			Move move = moves[i];
			int score = 0;
			Piece fromPiece = board.getPieceType(move.from);
			Piece toPiece = board.getPieceType(move.to);

			if (fromPiece != Piece::King)
			{
				score += Evaluation::pieceValues[fromPiece] / 100;
			}

			if (move.flag != Move::Flag::None)
			{
				score += 3;
			}

			if (move.isPromotion())
			{
				score += 10;
			}

			if (toPiece != Piece::None)
			{
				int victimValue = Evaluation::pieceValues[toPiece];
				int attackerValue = Evaluation::pieceValues[fromPiece];
				score += (10 * victimValue) - attackerValue;
			}

			score += moveHistory[(int)whiteToMove][move.from][move.to];

			moveScores[i] = score;
		}

		Sort::Quicksort(moves, moveScores, 0, moveSize - 1);
	}

	Move Search::searchBestMove(ChessBoard& chessBoard)
	{ 
		board = chessBoard;

		moveHistory = {};
		stats = { 0, 0, 0, 0, 0 };

		// almost the same performance with and without clear;
		//PVTable.clear();

		bool white = board.isWhiteToMove();

		// Start Iterative Deepening
		board.generateMoves();

		orderedMoves = board.getLegalMoves();
		movesSize = board.getMovesSize();
		moveScores = {};

		for (currentDepth = 1; currentDepth <= maxSearchDepth; currentDepth++)
		{
			if (abortSearch)
			{
				break;
			}

			search(currentDepth);

			if (evaluation >= 10000000 || evaluation <= -10000000)
			{
				break;
				std::cout << "Found Mate";
			}

			stats.depth = currentDepth;
		}

		// reset abort search for the next search;
		abortSearch = false;

		return orderedMoves[0];
	}

	void Search::search(int depth)
	{
		int alpha = Evaluation::NegInfinity;	// == -beta == -Inf
		int beta = Evaluation::PosInfinity;		// == -alpha == +Inf

		int bestScore = Evaluation::NegInfinity;
		//std::array<Move, Consts::MaxPossibleMoves> moves = orderedMoves;

		uint64_t zobristKey = board.getZobristKey();

		if (PVTable.contains(zobristKey))
		{
			Move bestMove = PVTable[zobristKey];
			auto it = std::find(orderedMoves.begin(), orderedMoves.end(), bestMove);

			if (it != orderedMoves.end())
			{
				std::swap(orderedMoves.front(), *it);
			}
		}

		for (int i = 0; i < movesSize; i++)
		{
			if (abortSearch)
			{
				return;
			}

			board.makeMove(orderedMoves[i]);

			int moveScore = -alphaBetaPruning(depth - 1, -beta, -alpha);

			board.unmakeMove();

			moveScores[i] = moveScore;
			
			if (moveScore > alpha)
			{
				alpha = moveScore;
				PVTable[zobristKey] = orderedMoves[i];
			}
		}

		evaluation = alpha;

		Sort::Quicksort(orderedMoves, moveScores, 0, movesSize - 1);
	}

	// alpha - maximum current player can get
	// beta - least opposite player can gain
	// NullCheck - flag to avoid doing NullMoves inside each other
	int Search::alphaBetaPruning(int depth, int alpha, int beta)
	{
		if (abortSearch)
		{
			return 0;
		}

		stats.nodesVisited++;

		if (depth == 0)
		{
			uint64_t zobristKey = board.getZobristKey();

			bool inTranspositionTable = transpositionTable.contains(zobristKey);
			int eval;

			if (inTranspositionTable)
			{
				stats.nodesTransposed++;
				eval = transpositionTable[zobristKey];
			}
			else
			{
				stats.nodesEvaluated++;
				eval = QuiescenceSearch(alpha, beta);
				//eval = Evaluation::EvaluatePosition(board);
				transpositionTable[zobristKey] = eval;
			}
				 
			return eval;
		}

		board.generateMoves();
		std::array<Move, Consts::MaxPossibleMoves> legalMoves = board.getLegalMoves();
		size_t movesSize = board.getMovesSize();

		GameState boardState = board.getGameState();
		bool gameOver = movesSize == 0 || boardState.isGameOver();

		if (gameOver)
		{
			stats.nodesEvaluated++;
			int gameOverEval = Evaluation::EvaluatePosition(board);

			// prioritize faster checkmates
			gameOverEval = gameOverEval > 0 ? gameOverEval + depth : gameOverEval - depth;
			return gameOverEval;
		}

		// Null move pruning
		if (depth >= 5 && !board.isKingInCheck(board.getMasks().threatMap))
		{
			// additionally reduce depth for faster calculations in endgame positions
			const int reducedDepth = 2;
			board.makeMove(Move {0, 0});
			int nullMoveScore = -alphaBetaPruning(depth - 1 - reducedDepth, -beta, -beta + 1);
			board.unmakeMove();

			// If null move score is >= beta, prune this branch
			if (nullMoveScore >= beta)
			{
				stats.nodesPruned++;
				return beta;
			}
		}

		// order moves
		orderMoves(legalMoves, movesSize);
		
		// try to find the best move from PVTable
		uint64_t zobristKey = board.getZobristKey();
		if (PVTable.contains(zobristKey))
		{
			Move bestMove = PVTable[zobristKey];
			auto it = std::find(legalMoves.begin(), legalMoves.end(), bestMove);

			if (it != legalMoves.end())
			{
				std::swap(legalMoves.front(), *it);
			}
		}
		

		for (int i = 0; i < movesSize; i++)
		{
			bool needsFullSearch = true;
			Move move = legalMoves[i];

			// assume that if move is bad at lower depth it is also bad at higher
			/*if (depth >= 3 && !board.isKingInCheck(board.getMasks().threatMap))
			{
				const int reducedDepth = 2;
				board.makeMove(Move{ 0, 0 });
				int nullMoveScore = -alphaBetaPruning(depth - 1 - reducedDepth, -alpha - 1, -alpha);
				board.unmakeMove();

				needsFullSearch = nullMoveScore >= beta;
			}*/

			board.makeMove(move);
			int moveScore = -alphaBetaPruning(depth - 1, -beta, -alpha);
			board.unmakeMove();

			if (moveScore > alpha)
			{
				alpha = moveScore;
				PVTable[zobristKey] = move;
			}

			if (alpha >= beta)
			{
				stats.nodesPruned++;

				bool white = (int)board.isWhiteToMove();
				int movePower = moveHistory[white][move.from][move.to];
				moveHistory[(int)white][move.from][move.to] = std::max(movePower, depth * depth);

				return Evaluation::PosInfinity;
			}
		}

		return alpha;
	}

	int Search::QuiescenceSearch(int alpha, int beta)
	{
		stats.nodesVisited++;
		stats.nodesEvaluated++;

		int staticEval = Evaluation::EvaluatePosition(board);
		alpha = std::max(alpha, staticEval);

		if (alpha >= beta)
		{
			stats.nodesPruned++;
			return alpha;
		}

		board.generateMoves(false);
		std::array<Move, Consts::MaxPossibleMoves> legalMoves = board.getLegalMoves();
		size_t movesSize = board.getMovesSize();

		orderMoves(legalMoves, movesSize);

		uint64_t zobristKey = board.getZobristKey();
		if (PVTable.contains(zobristKey))
		{
			Move bestMove = PVTable[zobristKey];
			auto it = std::find(legalMoves.begin(), legalMoves.end(), bestMove);

			if (it != legalMoves.end())
			{
				std::swap(legalMoves.front(), *it);
			}
		}

		for (int i = 0; i < movesSize; i++)
		{
			const Move& move = legalMoves[i];

			int captureValue = Evaluation::pieceValues[board.getPieceType(move.to)];
			int margin = Evaluation::pieceValues[Piece::Pawn];

			if (staticEval + captureValue + margin <= alpha)
			{
				continue;
			}

			board.makeMove(move);

			int moveScore = -QuiescenceSearch(-beta, -alpha);

			board.unmakeMove();

			if (moveScore > alpha)
			{
				alpha = moveScore;
				PVTable[zobristKey] = move;
			}

			if (alpha >= beta)  // Beta cutoff
			{
				stats.nodesPruned++;
				break;
			}
		}

		return alpha;
	}

	void Search::clearHistory()
	{
		//everything else is reseted on each search
		transpositionTable.clear();
		moveHistory = {};
		PVTable = {};
	}
}


#if 0

// Check what happens when the mate is found
int Search::Minimax(int depth, bool maximizingPlayer)
{
	if (depth == 0)
	{
		nodesEvaluated++;
		return Evaluation::EvaluatePositionStatic(board);
	}

	board.generateMoves();
	std::array<Move, Consts::MaxPossibleMoves> legalMoves = board.getLegalMoves();
	size_t movesSize = board.getMovesSize();
	int bestMoveScore;
	nodesVisited += movesSize;

	// if moveSize == 0 its gameOver, return static evaluation of position
	if (movesSize == 0)
	{
		nodesEvaluated++;
		return Evaluation::EvaluatePositionStatic(board);
	}

	if (maximizingPlayer)
	{
		bestMoveScore = Evaluation::NegInfinity;
		for (int i = 0; i < movesSize; i++)
		{
			board.makeMove(legalMoves[i]);
			int moveScore = Minimax(depth - 1, !maximizingPlayer);
			bestMoveScore = std::max(moveScore, bestMoveScore);
			board.unmakeMove();
		}
	}
	else
	{
		bestMoveScore = Evaluation::PosInfinity;
		for (int i = 0; i < movesSize; i++)
		{
			board.makeMove(legalMoves[i]);
			int moveScore = Minimax(depth - 1, !maximizingPlayer);
			bestMoveScore = std::min(moveScore, bestMoveScore);
			board.unmakeMove();
		}
	}

	return bestMoveScore;
}

int Search::Negamax(int depth)
{
	if (depth == 0)
	{
		nodesEvaluated++;
		return Evaluation::EvaluatePosition(board);
	}

	board.generateMoves();
	std::array<Move, Consts::MaxPossibleMoves> legalMoves = board.getLegalMoves();
	size_t movesSize = board.getMovesSize();

	// if moveSize == 0 its gameOver, return static evaluation of position
	if (movesSize == 0)
	{
		nodesEvaluated++;
		return Evaluation::EvaluatePositionStatic(board);
	}

	int bestMoveScore = Evaluation::NegInfinity;
	nodesVisited += movesSize;

	for (int i = 0; i < movesSize; i++)
	{
		board.makeMove(legalMoves[i]);
		int moveScore = -Negamax(depth - 1);
		bestMoveScore = std::max(moveScore, bestMoveScore);
		board.unmakeMove();
	}

	return bestMoveScore;
}

#endif