/*
* author: Himangshu Saikia, 2018-2021
* email : himangshu.saikia.iitg@gmail.com
*/
#include <algorithm>
#include "Xewali/features.h"
#include "mersenne.h"
#include "movepick.h"

namespace Features
{
	/*
	* MATERIAL
	* Calculate attack reachability of every piece on an empty board

	* Queen :
	Corners and edges (8x8, 28) : 21 = 588
	Corners and edges (6x6, 20) : 23 = 460
	Corners and edges (4x4, 12) : 25 = 300
	Corners and edges (2x2, 4)  : 27 = 108

	WEIGHTED AVERAGE OF QUEEN : 22.75

	* Rook :
	All Squares on board (64) : 14

	WEIGHTED AVERAGE OF ROOK : 14

	* Bishop:
	Corners and edges (8x8, 28) : 7 = 196
	Corners and edges (6x6, 20) : 9 = 180
	Corners and edges (4x4, 12) : 11 = 132
	Corners and edges (2x2, 4)  : 13 = 52

	WEIGHTED AVERAGE OF BISHOP : 8.75

	* Knight:
	Corner Squares (4)								: 2 = 8
	Edge squares adjacent to corner square (8)		: 3 = 24
	Other Edge squares (16)							: 4 = 64
	Corner Squares (6x6, 4)							: 4 = 16
	Edge squares (6x6, 16)							: 6 = 96
	Center Squares (4x4, 16)						: 8 = 128

	WEIGHTED AVERAGE OF KNIGHT : 5.25

	* Pawn :
	Rank (2-7, not rook files, 36)  : 2 = 72
	Rank (2-7,     rook files, 12)  : 1 attack squares = 12
	8th rank (1, any file, 8)       : average of rook/queen/knight/bishop moves at given square
	: Assuming this is a queen = 8 * 21 = 168

	WEIGHTED AVERAGE OF PAWN : 3.9375 (considering queen promotion)
	WEIGHTED AVERAGE OF PAWN : 1.3125 (not considering queen promotion)

	* King :
	Corner (4)					: 3 = 12
	Edges except corners (24)	: 5 = 120
	All other squares (36)		: 8 = 288
	(not considering castling as it's not an attack move)

	WEIGHTED AVERAGE OF KING : 6.5625

	*/

	//constexpr double QUEEN_VAL = 22.75;
	//constexpr double ROOK_VAL = 14.0;
	//constexpr double BISHOP_VAL = 8.75;
	//constexpr double KNIGHT_VAL = 5.25;
	//constexpr double PAWN_VAL = 1.3125;

	double Features::eval_static_material(const Position& pos)
	{
		return (QUEEN_VAL * (pos.queen_count(Color::WHITE) - pos.queen_count(Color::BLACK)) +
			ROOK_VAL * (pos.rook_count(Color::WHITE) - pos.rook_count(Color::BLACK)) +
			BISHOP_VAL * (pos.bishop_count(Color::WHITE) - pos.bishop_count(Color::BLACK)) +
			KNIGHT_VAL * (pos.knight_count(Color::WHITE) - pos.knight_count(Color::BLACK)) +
			PAWN_VAL * (pos.pawn_count(Color::WHITE) - pos.pawn_count(Color::BLACK))) / TOT_VAL;
	}

	/**
	* Calculates the influence of a side/color on a given square
	*
	* side to move = attacker
	* other side = defender
	*
	* 1. If a square is reachable ONLY by a piece of one color/side, that square is controlled
	* by that side.
	*
	* 2. If a square if defended ONLY by a piece with higher overall influence, say a queen
	* and attacked by a piece with at least one piece of lower overall influence, say a pawn, the square
	* is controlled by the attacker.
	*
	* 3. If the influence is tied, other pieces that attack/defend (also sliding motion
	* should be considered) are considered next IN ORDER OF INCREASING influence. Ties are broken
	* by rule 1 and 2.
	*/

	// It's not important how many squares a side controls
	// It's only important if a side controls squares with opponent pieces on them - i.e., if they can take a piece

	// For side to move
	// 0. If any piece can take any other piece which is undefended -> advantage
	// 1. if pawns can take minor pieces or higher which are defended -> advantage
	// 2. if minor pieces can take rooks or queens which are defended -> advantage
	// 3. if rooks can take queens which are defended -> advantage
	// 4. if any piece can take any other piece which is attacked more times than it is defended

	double Features::eval_static_attack_defense(const Position & pos)
	{
		// TODO : handle pinned pawns bishops, rooks and queens
		// easiest is to pass the legal move bitboard and calculate influence
		// of the side to move using it

		// attacker value list per square
		// square, color = the attacker value is determined by the overall influence of the piece and not by its type
		// this means that a rook can be valued less than a bishop or knight, for example, if it is less active, and an exchange sacrifice
		// may be a favorable option
		std::vector<int> attackers[64][2];
		for (Color color = WHITE; color <= BLACK; color++)
		{
			Bitboard bishops_and_queens = pos.bishops_and_queens(color);
			Bitboard rooks_and_queens = pos.rooks_and_queens(color);
			Bitboard occ = pos.occupied_squares();
			Bitboard pawns = pos.pawns(color);
			Bitboard pinned_pieces = pos.pinned_pieces(color);
			Bitboard all_pawn_attacks{ EmptyBoardBB };

			std::cout << "Color " << color << "\n";

			// Pawns
			for (int i = 0; i < pos.pawn_count(color); i++)
			{
				auto sq = pos.pawn_list(color, i);
				auto attacks = color == Color::WHITE ? pos.white_pawn_attacks(sq) : pos.black_pawn_attacks(sq);
				all_pawn_attacks |= attacks;

				while (attacks)
				{
					auto sq1 = pop_1st_bit(&attacks);
					attackers[sq1][color].emplace_back(1); // pawn value is 1
				}
			}

			// Queens
			for (int i = 0; i < pos.queen_count(color); i++)
			{
				auto sq = pos.queen_list(color, i);
				// including direct and x-ray attacks
				auto attacks = pos.queen_attacks(sq) | bishop_attacks_bb(sq, occ ^ bishops_and_queens) | rook_attacks_bb(sq, occ ^ rooks_and_queens) | (bishop_attacks_bb(sq, occ ^ pawns) & all_pawn_attacks);

				std::cout << "Q\n";
				print_bitboard(attacks);

				while (attacks)
				{
					auto sq1 = pop_1st_bit(&attacks);
					attackers[sq1][color].emplace_back(9); // queen value is 9
				}
			}

			// Rooks
			for (int i = 0; i < pos.rook_count(color); i++)
			{
				auto sq = pos.rook_list(color, i);
				// including direct and x-ray attacks
				auto attacks = pos.rook_attacks(sq) | rook_attacks_bb(sq, occ ^ rooks_and_queens);

				std::cout << "R\n";
				print_bitboard(attacks);

				while (attacks)
				{
					auto sq1 = pop_1st_bit(&attacks);
					attackers[sq1][color].emplace_back(5); // rook value is 5
				}
			}

			// Bishops
			for (int i = 0; i < pos.bishop_count(color); i++)
			{
				auto sq = pos.bishop_list(color, i);
				// including direct and x-ray attacks
				auto attacks = pos.bishop_attacks(sq) | bishop_attacks_bb(sq, occ ^ bishops_and_queens) | (bishop_attacks_bb(sq, occ ^ pawns) & all_pawn_attacks);

				std::cout << "B\n";
				print_bitboard(attacks);

				while (attacks)
				{
					auto sq1 = pop_1st_bit(&attacks);
					attackers[sq1][color].emplace_back(3); // bishop value is 3
				}
			}

			// Knights
			for (int i = 0; i < pos.knight_count(color); i++)
			{
				auto sq = pos.knight_list(color, i);
				if (bit_is_set(pinned_pieces, sq))
				{
					continue;
				}
				auto attacks = pos.knight_attacks(sq);

				std::cout << "N\n";
				print_bitboard(attacks);

				while (attacks)
				{
					auto sq1 = pop_1st_bit(&attacks);
					attackers[sq1][color].emplace_back(3); // knight value is 3
				}
			}

			// King
			auto king_attacks = pos.king_attacks(pos.king_square(color));
			while (king_attacks)
			{
				auto sq1 = pop_1st_bit(&king_attacks);
				attackers[sq1][color].emplace_back(2000); // king value is 2000
			}
		}

		int influence_white = 0;
		int influence_black = 0;
		for (auto sq = Square::SQ_A1; sq <= Square::SQ_H8; sq++)
		{
			auto& wa = attackers[sq][Color::WHITE];
			auto& ba = attackers[sq][Color::BLACK];

			// if square has one or no attacker from either side - 0 for both
			if (wa.empty() && ba.empty())
			{
				continue;
			}

			if (wa.size() == 1 && ba.size() == 1)
			{
				continue;
			}

			std::cout << "White attackers on square [" << square_to_string(sq) << "] : {";
			for (const auto& ww : wa)
			{
				std::cout << ww << " ";
			}
			std::cout << "}\n";

			std::cout << "Black attackers on square [" << square_to_string(sq) << "] : {";
			for (const auto& bb : ba)
			{
				std::cout << bb << " ";
			}
			std::cout << "}\n";

			// if a square has one or more attackers from one side and zero from the other
			// the side with attacker has influence
			if (!wa.empty() && ba.empty())
			{
				influence_white++;
				std::cout << "Square " << square_to_string(sq) << " is controlled by WHITE\n\n";
				continue;
			}
			if (wa.empty() && !ba.empty())
			{
				influence_black++;
				std::cout << "Square " << square_to_string(sq) << " is controlled by BLACK\n\n";
				continue;
			}

			// in case of one or more attackers from both sides
			// the side with the least value attacker has influence
			// if there are two equal valued attackers from both sides
			// we check the next values in a sorted sequence to break ties
			// if ties cannot be broken, influence is shared

			std::sort(wa.begin(), wa.end());
			std::sort(ba.begin(), ba.end());

			std::size_t wi = 0, bi = 0;
			while (wi < wa.size() || bi < ba.size())
			{
				if (wi == wa.size() || wa[wi] > ba[bi])
				{
					influence_black++;
					std::cout << "Square " << square_to_string(sq) << " is controlled by BLACK\n\n";
					break;
				}

				if (bi == ba.size() || wa[wi] < ba[bi])
				{
					influence_white++;
					std::cout << "Square " << square_to_string(sq) << " is controlled by WHITE\n\n";
					break;
				}

				// current lowest influences are equal
				// check the next piece(s)

				wi++;
				bi++;
			}
		}
		std::cout << "Net white influence " << influence_white << "\n";
		std::cout << "Net black influence " << influence_black << "\n";
		return double(influence_white - influence_black) / 64;
	}

	double eval_static(Position& pos)
	{
		int res;
		if (has_game_ended(pos, res))
		{
			return res;
		}

		return eval_static_material(pos);
		//return eval_influence(pos);
	}

	bool has_game_ended(Position& pos, int & result)
	{
		bool whiteToMove = (pos.side_to_move() == Color::WHITE);

		// 50 moves / threefold repetition / insufficient material to mate -> draw
		if (pos.is_draw())
		{
			result = 0;
			return true;
		}

		if (pos.is_mate())
		{
			result = whiteToMove ? -1 : 1;
			return true;
		}

		Move mlist[256];
		auto num_legal_moves = pos.all_legal_moves(mlist);

		if (num_legal_moves == 0)
		{
			// Stalemate
			result = 0;
			return true;
		}

		return false;
	}
}
