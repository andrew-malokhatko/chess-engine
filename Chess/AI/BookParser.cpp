
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>

#include "BookParser.h"
#include "ChessBoard.h"

namespace Chess::BookParser
{
	bool isFileName(char file)
	{
		return (file >= 'a' && file <= 'h');
	}

	Piece getPieceFromSymbol(char piece)
	{
		switch (piece)
		{
		case 'K':
			return Piece::King;

		case 'Q':
			return Piece::Queen;

		case 'N':
			return Piece::Knight;

		case 'B':
			return Piece::Bishop;

		case 'R':
			return Piece::Rook;
		}

		return Piece::None;
	}

	Move moveFromAlgebraic(std::string& algebraicMove, ChessBoard board)
	{
		const std::array<char, 4> chars = { '+', '#', 'x', '-' };

		board.generateMoves();
		std::vector<Move> legalMoves = board.getLegalMovesAsVector();

		bool whiteToMove = board.isWhiteToMove();

		assert(legalMoves.size() > 0);

		// Remove all unnesseccary characters
		for (int i = 0; i < chars.size(); i++)
		{
			algebraicMove.erase(std::remove(algebraicMove.begin(), algebraicMove.end(), chars[i]), algebraicMove.end());
		}

		// Test all moves
		for (const Move& move : legalMoves)
		{
			Piece movePieceType = board.getPieceType(move.from);

			std::string fromCoord = ChessBoard::indexToCoord(move.from);
			std::string toCoord = ChessBoard::indexToCoord(move.to);

			if (algebraicMove == "OO")
			{
				if (movePieceType == Piece::King && move.to - move.from == 2)
				{
					return move;
				}
			}
			else if (algebraicMove == "OOO")
			{
				if (movePieceType == Piece::King && move.to - move.from == -2)
				{
					return move;
				}
			}
			// OMMITED PROMOTIONS !!!!!!!!!!
			// If move starts from file name its a pawn move + file names are the same
			else if (algebraicMove[0] == fromCoord[0])
			{
				if (movePieceType != Piece::Pawn)
				{
					continue;
				}

				char targetFile = algebraicMove[algebraicMove.size() - 2];
				char targetRank = algebraicMove[algebraicMove.size() - 1];

				if (std::string(1, targetFile) + targetRank == toCoord)
				{
					return move;
				}
			}
			// Normal moves 
			else
			{
				char movePieceChar = algebraicMove[0];
				Piece algebraicPieceType = getPieceFromSymbol(movePieceChar);

				if (algebraicPieceType != movePieceType)
				{
					continue;
				}

				char targetFile = algebraicMove[algebraicMove.size() - 2];
				char targetRank = algebraicMove[algebraicMove.size() - 1];

				if (std::string(1, targetFile) + targetRank == toCoord)
				{
					// If 2 piece of the type which are on the same rank/file can move to target square
					if (algebraicMove.size() == 4)
					{
						char specification = algebraicMove[1];
						// if rank is specified
						if (std::isdigit(specification))
						{
							// fromCoord e.g e5 (file, rank)
							if (specification == fromCoord[1])
							{
								return move;
							}
						}
						// if file specified
						else
						{
							if (specification == fromCoord[0])
							{
								return move;
							}
						}

						continue;
					}

					return move;
				}
			}
		}

		// move was not found
		std::cout << algebraicMove << std::endl;
		assert(0);

		return Move{ 0, 0 };
	}

	std::vector<Move> movesFromAlgebraic(const std::string& gameAlgebraic)
	{
		std::istringstream gameStream(gameAlgebraic);
		std::vector<std::string> game;
		game.reserve(MaxPlyCount);
		std::string entry;

		ChessBoard gameBoard;
		std::vector<Move> moves;
		moves.reserve(MaxPlyCount);

		// create array from individual algebraic moves
		while (gameStream >> entry && game.size() < MaxPlyCount)
		{
			//trim(entry);
			game.push_back(entry);
		}

		// Iterate over the array of string algebraic moves and convert them to actual Moves 
		for (int i = 0; i < game.size(); i++)
		{
			entry = game[i];

			if (MaxPlyCount == moves.size())
			{
				break;
			}

			// || entry == '\n'
			if (entry == "1/2-1/2" || entry == "1-0" || entry == "0-1")
			{
				break;
			}

			Move move = moveFromAlgebraic(entry, gameBoard);

			gameBoard.makeMove(move);

			moves.emplace_back(move);
		}

		return moves;
	}

	Book movesFromPGN(const std::string& gamesPGNFile)
	{
		Book openingBook;

		std::ifstream gamesPGN(gamesPGNFile);
		std::string gameString;

		int countLoadedGames = 0;

		std::vector<std::vector<Move>> gamesByMoves;
		gamesByMoves.reserve(300000);

		while (std::getline(gamesPGN, gameString, '\n'))
		{
			gamesByMoves.emplace_back(movesFromAlgebraic(gameString));
		}

		for (const std::vector<Move>& game : gamesByMoves)
		{
			ChessBoard chessBoard;
			uint64_t curZobristKey;

			countLoadedGames++;

			for (const Move& move : game)
			{
				curZobristKey = chessBoard.getZobristKey();
				openingBook.insert(curZobristKey, move);

				chessBoard.makeMove(move);
			}
		}

		std::cout << "Loaded book moves. " << "Loaded " << countLoadedGames << " Games" << std::endl;
		writeBookTo(openingBook, "Hello.txt");

		return openingBook;
	}

	Book movesFromBook(const std::string& bookMoveFile)
	{
		Book openingBook;

		std::ifstream bookFile(bookMoveFile);
		std::string line;

		std::string moveData;
		std::string hashStr;
		std::unordered_map<Move, int> movesFromKey;

		while (std::getline(bookFile, line, '\n'))
		{
			movesFromKey.clear();

			std::istringstream ss(line);
			
			std::getline(ss, hashStr, '#');
			uint64_t zobristKey = std::stoull(hashStr);

			while (getline(ss, moveData, '#'))
			{
				auto colonPos = moveData.find(':');

				int encodedMove = std::stoi(moveData.substr(0, colonPos));
				Move move = Move::decode(encodedMove);

				int count = std::stoi(moveData.substr(colonPos + 1));

				movesFromKey[move] = count;
			}

			openingBook.insertMap(zobristKey, movesFromKey);
		}

		return openingBook;
	}

	void writeBookTo(const Book& openingBook, const char* fileName)
	{
		const auto& book = openingBook.getBook();

		std::ofstream outputFile(fileName);
		std::string output;

		for (const auto& position : book)
		{
			output.append(std::to_string(position.first));

			for (const auto& move : position.second)
			{
				output.push_back('#');

				output.append(std::to_string(move.first.encode()));
				output.push_back(':');
				output.append(std::to_string(move.second));

			}

			output.push_back('\n');
		}
		outputFile.write(output.c_str(), output.size());
	}
}