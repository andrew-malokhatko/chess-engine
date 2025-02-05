#pragma once
#include <compare>

namespace Chess
{
	class Piece
	{
	private:


	public:
		static constexpr int White = 0;
		static constexpr int Black = 8;

		static constexpr int Rook = 1;
		static constexpr int Knight = 2;
		static constexpr int Bishop = 3;
		static constexpr int Queen = 4;
		static constexpr int King = 5;
		static constexpr int Pawn = 6;

		enum PieceType
		{
			None,
			WhiteRook = Rook | White,
			WhiteKnight = Knight | White,
			WhiteBishop = Bishop | White,
			WhiteQueen = Queen | White,
			WhiteKing = King | White,
			WhitePawn = Pawn | White,

			BlackRook = Rook | Black,
			BlackKnight = Knight | Black,
			BlackBishop = Bishop | Black,
			BlackQueen = Queen | Black,
			BlackKing = King | Black,
			BlackPawn = Pawn | Black,
		};

	private:
		PieceType value = None;

	public:
		Piece() = default;
		Piece(PieceType piece) :
			value{ piece }
		{
		}
		Piece(int piece) :
			value{ static_cast<PieceType>(piece) }
		{
		}

		[[nodiscard]] bool isBlack() const
		{
			return value & Piece::Black;
		}

		[[nodiscard]] bool isWhite() const
		{
			return !isBlack();
		}

		bool operator==(const Piece& other) const
		{
			return value == other.value;
		}
		bool operator==(const Piece::PieceType& other) const
		{
			return value == other;
		}
		bool operator==(const int& other) const
		{
			return value == other;
		}

		// automatic conversion operator, piece would always be returned as PieceType
		operator PieceType() const
		{
			return value;
		}
	};
}

// redefine hash funtion for usage in unordered_map
namespace std
{
	template<>
	struct hash<Chess::Piece>
	{
		std::size_t operator()(const Chess::Piece& piece) const
		{
			return std::hash<int>{}(static_cast<int>(piece));
		}
	};
}