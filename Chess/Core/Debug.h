#pragma once

#include <cstdint>

namespace Chess
{
	struct DebugMasks
	{
		uint64_t threatmap;
		uint64_t pinMask;
		uint64_t checkMask;
	};

	struct SearchStats
	{
		int depth;
		int nodesVisited;
		int nodesEvaluated;
		int nodesPruned;
		int nodesTransposed;
	};

	struct DebugData
	{
		DebugMasks masksWhite;
		DebugMasks masksBlack;

		std::unordered_map<Move, int> book;
		SearchStats searchStats;
	};

	
}