#pragma once

#include <string>

#include "Book.h"

namespace Chess::BookParser
{
	static int MaxPlyCount = 10;

	Book movesFromPGN(const std::string& gamesPGNFile);
	Book movesFromBook(const std::string& bookFile);

	void writeBookTo(const Book& openingBook, const char* fileName);
}