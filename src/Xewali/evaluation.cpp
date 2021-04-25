/*
* author: Himangshu Saikia, 2018-2021
* email : himangshu.saikia.iitg@gmail.com
*/
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Xewali/evaluation.h"
#include "mersenne.h"
#include "movepick.h"

namespace Evaluation
{
	void load_games(Book & book, const std::string & game_file)
	{
		std::ifstream file;
		file.open(game_file);

		if (file.is_open())
		{
			std::string line;
			int games = 0;
			while (std::getline(file, line))
			{
				games++;

				std::string move_string;
				std::istringstream iss(line);
				Position pos = Position(StartPosition);
				UndoInfo u;
				while (iss >> move_string)
				{
					auto move = move_from_string(pos, move_string);
					auto key = pos.get_key();
					book[key].insert(move);
					pos.do_move(move, u);

				}
			}

			std::cout << games << " Games loaded \n";
			std::cout << book.size() << " book moves recorded\n";

			file.close();
		}
		else
		{
			std::cout << "Book could not be loaded\n";
		}
	}

	double eval(Position& pos)
	{
		int res;
		if (has_game_ended(pos, res))
		{
			return res;
		}

		Bitboard attack_bb[2][7];
		int material[2];

		for (Color color = WHITE; color <= BLACK; color++)
		{
			material[color] = 0;
			for (auto p = PieceType::PAWN; p <= KING; p++)
			{
				attack_bb[color][p] = EmptyBoardBB;
			}
		}

		for (Color color = WHITE; color <= BLACK; color++)
		{
			// Pawns
			for (int i = 0; i < pos.pawn_count(color); i++)
			{
				auto sq = pos.pawn_list(color, i);
				auto attacks = color == Color::WHITE ? pos.white_pawn_attacks(sq) : pos.black_pawn_attacks(sq);
				attack_bb[color][PieceType::PAWN] |= attacks;
				material[color] += PAWN_VAL;
				material[color] += color == Color::WHITE ? WhitePawnTable[sq] : BlackPawnTable[sq];
			}

			// Bishops
			for (int i = 0; i < pos.bishop_count(color); i++)
			{
				auto sq = pos.bishop_list(color, i);
				auto attacks = pos.bishop_attacks(sq) & (~attack_bb[opposite_color(color)][PieceType::PAWN]);
				attack_bb[color][PieceType::BISHOP] |= attacks;
				material[color] += BISHOP_VAL;
				material[color] += color == Color::WHITE ? WhiteBishopTable[sq] : BlackBishopTable[sq];
			}

			// Knights
			for (int i = 0; i < pos.knight_count(color); i++)
			{
				auto sq = pos.knight_list(color, i);
				auto attacks = pos.knight_attacks(sq) & (~attack_bb[opposite_color(color)][PieceType::PAWN]);
				attack_bb[color][PieceType::KNIGHT] |= attacks;
				material[color] += KNIGHT_VAL;
				material[color] += color == Color::WHITE ? WhiteKnightTable[sq] : BlackKnightTable[sq];
			}

			// Rooks
			for (int i = 0; i < pos.rook_count(color); i++)
			{
				auto sq = pos.rook_list(color, i);
				auto attacks = pos.rook_attacks(sq) & (~attack_bb[opposite_color(color)][PieceType::PAWN]);
				attack_bb[color][PieceType::ROOK] |= attacks;
				material[color] += ROOK_VAL;
				material[color] += color == Color::WHITE ? WhiteRookTable[sq] : BlackRookTable[sq];
			}

			// Queens
			for (int i = 0; i < pos.queen_count(color); i++)
			{
				auto sq = pos.queen_list(color, i);
				auto attacks = pos.queen_attacks(sq) & (~attack_bb[opposite_color(color)][PieceType::PAWN] | ~attack_bb[opposite_color(color)][PieceType::KNIGHT] | ~attack_bb[opposite_color(color)][PieceType::BISHOP] | ~attack_bb[opposite_color(color)][PieceType::ROOK]);
				attack_bb[color][PieceType::QUEEN] |= attacks;
				material[color] += QUEEN_VAL;
				material[color] += color == Color::WHITE ? WhiteQueenTable[sq] : BlackQueenTable[sq];
			}

			// King
			attack_bb[color][PieceType::KING] = pos.king_attacks(pos.king_square(color)) & (~attack_bb[opposite_color(color)][PieceType::PAWN] | ~attack_bb[opposite_color(color)][PieceType::KNIGHT] | ~attack_bb[opposite_color(color)][PieceType::BISHOP] | ~attack_bb[opposite_color(color)][PieceType::ROOK] | ~attack_bb[opposite_color(color)][PieceType::QUEEN]);

			// If material has reduced significantly, the end game stage is reached
			// The Kings should venture out to the middle
			if (material[Color::WHITE] < 2000 && material[Color::BLACK] < 2000)
			{
				material[Color::WHITE] += WhiteKingEGTable[pos.king_square(Color::WHITE)];
				material[Color::BLACK] += BlackKingEGTable[pos.king_square(Color::BLACK)];
			}
			else
			{
				material[Color::WHITE] += WhiteKingMGTable[pos.king_square(Color::WHITE)];
				material[Color::BLACK] += BlackKingMGTable[pos.king_square(Color::BLACK)];
			}

		}

		int influence[2];

		for (Color color = Color::WHITE; color <= Color::BLACK; color++)
		{
			influence[color] = 0;
			for (auto p = PieceType::PAWN; p <= PieceType::KING; p++)
			{
				influence[color] += count_1s(attack_bb[color][p]);
			}
		}

		return double(material[Color::WHITE] - material[Color::BLACK]) + 10 * std::log(double(influence[Color::WHITE]) / double(influence[Color::BLACK]));
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
			result = whiteToMove ? -mateEval : mateEval;
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
