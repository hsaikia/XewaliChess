/*
* author: Himangshu Saikia, 2018-2021
* email : himangshu.saikia.iitg@gmail.com
*/

#include <algorithm>
#include <ctime>
#include <set>
#include <map>
#include <fstream>
#include <ctime>
#include <random>
#include "Chess/mersenne.h"
#include "Chess/movepick.h"
#include "Xewali/ab_id_engine.h"

namespace AbIterDeepEngine
{
	void init()
	{
		init_mersenne();
		init_direction_table();
		init_bitboards();
		Position::init_zobrist();
		Position::init_piece_square_tables();
		MovePicker::init_phase_table();
	}

	void set_position(Position& pos, const std::string & fen, const std::vector<std::string>& moves)
	{
		pos = Position(fen);

		for (auto& move : moves)
		{
			UndoInfo u;
			Position posTemp = pos;
			pos.do_move(move_from_string(posTemp, move), u);
		}
	}

	struct MoveNode;

	struct MoveNode
	{
		Move move = Move::MOVE_NONE;
		double eval = 0.;
		std::vector<std::shared_ptr<MoveNode>> order_next;
		std::set<Move> keys_next;
	};

	void print_tree(std::shared_ptr<MoveNode> move_node, const std::string& old, int& count, std::ofstream& file)
	{
		std::string pre = "";
		if (move_node->move != MOVE_NONE && std::abs(move_node->eval) < 2)
		{
			pre = old + " " + move_to_string(move_node->move);
			file << pre << "[" << move_node->eval << "]\n";
			count++;
		}
		for (size_t i = 0; i < move_node->order_next.size(); i++)
		{
			print_tree(move_node->order_next[i], pre, count, file);
		}
	}

	void populate_next_moves(std::shared_ptr<MoveNode> node, Position& pos, bool only_captures_and_checks)
	{
		bool is_root_move_capture = false;
		UndoInfo u;
		if (node->move != MOVE_NONE)
		{
			is_root_move_capture = pos.move_is_capture(node->move);
			pos.do_move(node->move, u);
		}

		Move move_list[256];
		int num_legal_moves = pos.all_legal_moves(move_list);
		bool white_to_move = pos.side_to_move() == Color::WHITE;
		for (size_t i = 0; i < num_legal_moves; i++)
		{
			const auto& move = move_list[i];

			if (node->keys_next.find(move) != node->keys_next.end())
			{
				// move already exists
				continue;
			}

			// if only captures need to be taken
			// 1. first move should be a capture
			// 2. second move should be capture at the same square
			// TODO : include checks as well
			if (only_captures_and_checks && !(is_root_move_capture && move_to(node->move) == move_to(move)))
			{
				// only return captures
				continue;
			}

			auto child_ptr = std::make_shared<MoveNode>();
			child_ptr->move = move;
			child_ptr->eval = white_to_move ? std::numeric_limits<double>::lowest() : (std::numeric_limits<double>::max)();
			node->order_next.emplace_back(child_ptr);
			node->keys_next.insert(move);
		}

		if (node->move != MOVE_NONE)
		{
			pos.undo_move(node->move, u);
		}
	}

	void order_moves(std::shared_ptr<MoveNode> node, const bool white_to_move)
	{
		// sort the moves
		if (white_to_move)
		{
			std::sort(node->order_next.begin(), node->order_next.end(),
				[](std::shared_ptr<MoveNode> a, std::shared_ptr<MoveNode> b)
			{
				return a->eval > b->eval;
			});
		}
		else
		{
			std::sort(node->order_next.begin(), node->order_next.end(),
				[](std::shared_ptr<MoveNode> a, std::shared_ptr<MoveNode> b)
			{
				return a->eval < b->eval;
			});
		}
	}

	void minimax(std::shared_ptr<MoveNode> node, Position& pos, double alpha, double beta, int depth, std::map<Key, std::pair<int, double> >& transposition_table, int& transpositions)
	{
		populate_next_moves(node, pos, depth == 0);
		UndoInfo u;

		// make move
		if (node->move != MOVE_NONE)
		{
			pos.do_move(node->move, u);
		}

		// get the zobrist hash key
		auto key = pos.get_key();
		auto table_val = transposition_table.find(key);

		// position should have been seen before and evaluated at an equal or higher depth
		// if this is the case, the position need not be evaluated again
		if (table_val != transposition_table.end() && table_val->second.first >= depth)
		{
			node->eval = table_val->second.second;
			transpositions++;
		}
		else
		{
			// terminal position reached, call static evaluation function
			if (node->order_next.empty())
			{
				node->eval = Evaluation::eval(pos);
				// terminal position evaluated
				transposition_table[pos.get_key()] = std::pair<int, double>(depth, node->eval);
			}
			else
			{
				// recurse
				bool white_to_move = pos.side_to_move() == Color::WHITE;
				node->eval = white_to_move ? std::numeric_limits<double>::lowest() : (std::numeric_limits<double>::max)();

				for (int i = 0; i < node->order_next.size(); i++)
				{
					const auto move_node = node->order_next[i];
					minimax(move_node, pos, alpha, beta, (std::max)(depth - 1, 0), transposition_table, transpositions);
					double eval = move_node->eval;

					if (white_to_move)
					{
						node->eval = (std::max)(node->eval, eval);
						alpha = (std::max)(alpha, eval);
					}
					else
					{
						node->eval = (std::min)(node->eval, eval);
						beta = (std::min)(beta, eval);
					}

					if (beta < alpha) // cutoff reached - stop looking at other moves
					{
						break;
					}
				}

				// non-terminal position evaluated
				transposition_table[pos.get_key()] = std::pair<int, double>(depth, node->eval);
				// order child moves
				order_moves(node, white_to_move);
			}
		}

		// undo move
		if (node->move != MOVE_NONE)
		{
			pos.undo_move(node->move, u);
		}
	}

	void print_move_sequence(std::shared_ptr<MoveNode> move_node)
	{
		std::cout << "[" << move_node->eval << "] ";
		while (move_node)
		{
			std::cout << move_to_string(move_node->move) << " ";
			move_node = move_node->order_next.empty() ? nullptr : move_node->order_next[0];
		}
		std::cout << "\n";
	}

	std::string play_move(Position& pos, double& eval, const Evaluation::Book& book)
	{
		// Try to find a random move from the book
		std::mt19937 rand_gen(time(NULL));
		const Key pos_key = pos.get_key();
		std::vector<Move> book_moves;
		if (book.find(pos_key) != book.end())
		{
			const auto& move_set = book.find(pos_key)->second;
			book_moves = std::vector<Move>(move_set.begin(), move_set.end());
		}

		// see if there are more than one choices
		if (book_moves.size() > 1)
		{
			std::uniform_int_distribution<std::size_t> rand_idx(0, book_moves.size() - 1);
			auto idx = rand_idx(rand_gen);
			std::cout << "Choosing random move from " << book_moves.size() << " moves\n";
			auto choosen_move = book_moves[idx];
			return move_to_string(choosen_move);
		}

		// Iterative Deepening searches the move tree by iteratively increasing 
		// the depth to a max depth. This may sound counterintuitive, since
		// we end up evaluating many of the same positions again, but the move ordering 
		// obtained during previous searches at lower depths can help in pruning 
		// more branches at higher depths.

		clock_t start = std::clock();

		//constexpr int max_depth = 100;
		int depth = 1;
		std::map<Key, std::pair<int, double> > transposition_table;

		std::shared_ptr<MoveNode> no_move = std::make_shared<MoveNode>();

		for (depth = 1; ; depth++)
		{
			int transpositions = 0;
			transposition_table.clear();
			minimax(no_move, pos, std::numeric_limits<double>::lowest(), (std::numeric_limits<double>::max)(), depth, transposition_table, transpositions);

			if (!no_move->order_next.empty())
			{
				// If mate found, no need to evaluate deeper
				if (std::abs(no_move->order_next[0]->eval) == Evaluation::mateEval)
				{
					break;
				}

				// if there's only one move, no point recursing to higher depths
				if (no_move->order_next.size() == 1)
				{
					break;
				}
			}

			std::cout << "---- Search at Depth " << depth << " completed. ----\n";
			std::cout << transposition_table.size() << " Positions evaluated\n";
			std::cout << transpositions << " Transpositions\n";

			// do not search at higher depths if more than a second has elapsed
			if ((std::clock() - start) / (double)CLOCKS_PER_SEC > 1.0)
			{
				break;
			}
		}

		//std::ofstream file;
		//file.open("Lines.txt");

		//int lines_seen = 0;
		//print_tree(no_move, "", lines_seen, file);
		//file << "Lines seen " << lines_seen << "\n";

		//file.close();

		for (const auto next_move_ptr : no_move->order_next)
		{
			std::cout << "Eval of move [" << move_to_string(next_move_ptr->move) << "] is {" << next_move_ptr->eval << "}\n";
		}

		if (no_move->order_next.empty())
		{
			eval = 0.;
			return "";
		}

		eval = no_move->order_next[0]->eval;
		return move_to_string(no_move->order_next[0]->move);
	}
}