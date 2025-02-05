#pragma once

#include <unordered_map>

#include "ChessBoard.h"
#include "Debug.h"

namespace Chess
{
	struct VoidHasher
	{
		size_t operator()(uint64_t v) const
		{
			return v;
		}
	};
	
	class Search
	{
		static constexpr int maxSearchDepth = 30;
		int currentDepth = 0;
		bool abortSearch = false;

		SearchStats stats;

		int evaluation = 0;
		
		std::array<Move, Consts::MaxPossibleMoves> orderedMoves = {};
		std::array<int, Consts::MaxPossibleMoves> moveScores = {};
		size_t movesSize = 0;

		// Store already evaluated positions
		std::unordered_map<uint64_t, int, VoidHasher> transpositionTable = {};

		// Store moves that caused alpha beta cutoff for white and black (killer moves)
		std::array<std::array<std::array<int, 64>, 64>, 2> moveHistory = {};

		// Store best moves for each encountered position
		std::unordered_map<uint64_t, Move> PVTable = {};

		ChessBoard board;

	public:
		Search()
		{
			transpositionTable.reserve(10'000'000);
			stats = { 0, 0, 0, 0, 0 };
		}

		Move searchBestMove(ChessBoard& board);
		void search(int depth);
		void orderMoves(std::array<Move, Consts::MaxPossibleMoves>& moves, int moveSize);

		SearchStats getSearchStats() const { return stats; };

		void stopSearch() { abortSearch = true; };
		int getEvaluation() { return evaluation; };

		//int Minimax(int depth, bool maximizingPlayer);
		//int Negamax(int depth);
		int alphaBetaPruning(int depth, int alpha, int beta);
		int QuiescenceSearch(int alpha, int beta);

		void clearHistory();

	};
}