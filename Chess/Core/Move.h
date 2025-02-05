#pragma once

namespace Chess
{
	class Move
	{
	private:
		static constexpr uint16_t FROM = 0b111111;	// bits 0 - 5
		static constexpr uint16_t TO = FROM << 6;	// bits 6 - 11
		static constexpr uint16_t FLAG = FROM << 12;	// bits 12...

	public:
		// Values of Pieces are equal to Promotion ones (DO NOT CHANGE)
		enum Flag
		{
			None,
			PromotionRook,
			PromotionKnight,
			PromotionBishop,
			PromotionQueen,
			CastlingKingside,
			CastlingQueenside,
			EnPassant,
			DoublePush
		};

		int from = 0;
		int to = 0;
		Flag flag = None;

		auto operator<=>(const Move& other) const = default;

		[[nodiscard]] bool isNullMove() const
		{
			return from == 0 && to == 0;
		}

		[[nodiscard]] bool isPromotion() const
		{
			return flag == PromotionBishop || flag == PromotionKnight || flag == PromotionQueen || flag == PromotionRook;
		}

		[[nodiscard]] bool isDoublePush() const
		{
			return flag == DoublePush;
		}

		[[nodiscard]] bool isEnPassant() const
		{
			return flag == EnPassant;
		}

		[[nodiscard]] bool isCastling() const
		{
			return flag == CastlingKingside || flag == CastlingQueenside;
		}

		uint16_t encode() const
		{
			return static_cast<uint16_t>(from | (to << 6) | (flag << 12));
		}

		static Move decode(uint16_t encodedMove)
		{
			return Move{ encodedMove & FROM, (encodedMove & TO) >> 6, static_cast<Flag>((encodedMove & FLAG) >> 12) };
		}
	};
}

namespace std
{
	template<>
	struct hash<Chess::Move>
	{
		std::size_t operator()(const Chess::Move& move) const
		{
			return std::hash<uint16_t>{}(move.encode());
		}
	};
}