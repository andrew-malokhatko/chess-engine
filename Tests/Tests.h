#pragma once

namespace Chess::Test
{
	struct TestPosition
	{
		int depth;
		int nodes;
		std::string fen;
	};

	extern const std::vector<TestPosition> testGithub;
	extern const std::vector<TestPosition> testDefault;

	void testMoveGeneration(const std::vector<TestPosition>& positions);
}