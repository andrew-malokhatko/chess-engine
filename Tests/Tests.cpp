#include <chrono>
#include <iostream>
#include <future>
#include <thread>

#include "Tests.h"
#include "ChessBoard.h"

namespace Chess::Test
{
	const std::vector<TestPosition> testGithub = {
	   TestPosition {1, 8, "r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2"},
	   TestPosition {1, 8, "8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3"},
	   TestPosition {1, 19, "r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2"},
	   TestPosition {1, 5, "r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2"},
	   TestPosition {1, 44, "2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2"},
	   TestPosition {1, 39, "rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9"},
	   TestPosition {1, 9, "2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4"},
	   TestPosition {3, 62379, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
	   TestPosition {3, 89890, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"},
	   TestPosition {6, 1134888, "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1"},
	   TestPosition {6, 1015133, "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1"},
	   TestPosition {6, 1440467, "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1"},
	   TestPosition {6, 661072, "5k2/8/8/8/8/8/8/4K2R w K - 0 1"},
	   TestPosition {6, 803711, "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"},
	   TestPosition {4, 1274206, "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1"},
	   TestPosition {4, 1720476, "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1"},
	   TestPosition {6, 3821001, "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"},
	   TestPosition {5, 1004658, "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1"},
	   TestPosition {6, 217342, "4k3/1P6/8/8/8/8/K7/8 w - - 0 1"},
	   TestPosition {6, 92683, "8/P1k5/K7/8/8/8/8/8 w - - 0 1"},
	   TestPosition {6, 2217, "K1k5/8/P7/8/8/8/8/8 w - - 0 1"},
	   TestPosition {7, 567584, "8/k1P5/8/1K6/8/8/8/8 w - - 0 1"},
	   TestPosition {4, 23527, "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1"}
	};

	//https://www.chessprogramming.org/Perft_Results
	const std::vector<TestPosition> testDefault = {
		TestPosition {5, 193690690, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "},
		TestPosition {5, 15833292, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
		TestPosition {5, 89941194, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "},
		TestPosition {6, 119060324, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
		TestPosition {5, 164075551, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 "},
		TestPosition {7, 178633661, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "},
		TestPosition {7, 178633661, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "},
		TestPosition {7, 178633661, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "},
	};

	void testMoveGeneration(const std::vector<TestPosition>& positions)
	{
		std::cout << "Starting Move Generation Test.." << std::endl << std::endl;

		auto start_time = std::chrono::high_resolution_clock::now();
		int success = 0;
		int positionCount = 0;


		auto func = [&positionCount](TestPosition position)
			{
				auto start_time = std::chrono::high_resolution_clock::now();

				ChessBoard board;
				board.loadPosFromFen(position.fen);
				int result = board.perft(position.depth);

				auto diff = std::chrono::high_resolution_clock::now() - start_time;
				std::cout << "CalcTime, fen " << position.fen << ", time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff) << " ms\n";

				return result;
			};


#define NOPARALLEL

#ifdef PARALLEL
		std::vector<std::future<int>> tasks;
		tasks.reserve(positions.size());

		for (const auto& position : positions)
		{
			auto f = std::async(std::launch::async, func, position);
			tasks.push_back(std::move(f));
		}

		for (size_t i = 0; i < tasks.size(); i++)
		{
			int result = tasks[i].get();
			positionCount += result;

			if (result == positions[i].nodes)
			{
				std::cout << "\033[32mPassed:\033[0m " << positions[i].fen << " - " << result << " / " << positions[i].nodes << " were found" << std::endl;
				success++;
			}
			else
			{
				std::cout << "\033[31mError:\033[0m " << positions[i].fen << " - " << result << " / " << positions[i].nodes << " were found" << std::endl;
			}
		}
#else
		for (const auto& position : positions)
		{
			auto result = func(position);
			positionCount += result;

			if (result == position.nodes)
			{
				std::cout << "\033[32mPassed:\033[0m " << position.fen << " - " << result << " / " << position.nodes << " were found" << std::endl;
				success++;
			}
			else
			{
				std::cout << "\033[31mError:\033[0m " << position.fen << " - " << result << " / " << position.nodes << " were found" << std::endl;
			}
		}
#endif

		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;
		auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(time);

		std::cout << std::endl << "Test finished in " << timeMs << " milliseconds" << "Calculated " << positionCount << " positions" << std::endl;
		std::cout << success << " out of " << positions.size() << " position were calculated correctly" << std::endl;
	}
}