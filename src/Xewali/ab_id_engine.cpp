/*
* author: Himangshu Saikia, 2018
* email : himangshu.saikia.iitg@gmail.com
*/

#include <algorithm>
#include <ctime>
#include <set>
#include <fstream>
#include "Chess/mersenne.h"
#include "Chess/movepick.h"
#include "Xewali/ab_id_engine.h"
#include "Xewali/features.h"

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
		//std::cout << "Print Tree called\n";
		std::string pre = "";
		if (move_node->move == MOVE_NONE)
		{
			//
		}
		else
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

			//if (only_captures_and_checks && !pos.move_is_capture(move) && !pos.move_is_check(move))
			//{
			//	// only captures and check moves need to be added
			//	continue;
			//}

			// if only captures need to be taken
			// 1. first move should be a capture
			// 2. second move should be capture at the same square
			if (only_captures_and_checks && !(is_root_move_capture && move_to(node->move) == move_to(move)))
			{
				// only return captures
				continue;
			}

			// sometimes captures are not high value captures
			// looking into these lines are wasteful and can result in wrong scores
			// being propagated to the top

			//std::cout << "Adding move " << move_to_string(move) << "\n";

			auto child_ptr = std::make_shared<MoveNode>();
			child_ptr->move = move;
			child_ptr->eval = white_to_move ? std::numeric_limits<double>::lowest() : (std::numeric_limits<double>::max)();
			node->order_next.emplace_back(child_ptr);
			node->keys_next.insert(move);
		}

		//print_tree(node, "");

		if (node->move != MOVE_NONE)
		{
			pos.undo_move(node->move, u);
		}
	}

	void minimax(std::shared_ptr<MoveNode> node, Position& pos, double alpha, double beta, int depth)
	{
		populate_next_moves(node, pos, depth == 0);
		//print_tree(node, "");
		UndoInfo u;
		if (node->order_next.empty())
		{
			if (node->move != MOVE_NONE)
			{
				pos.do_move(node->move, u);
			}

			node->eval = Features::eval_static(pos);

			if (node->move != MOVE_NONE)
			{
				pos.undo_move(node->move, u);
			}

			return;
		}

		if (node->move != MOVE_NONE)
		{
			pos.do_move(node->move, u);
		}

		bool white_to_move = pos.side_to_move() == Color::WHITE;
		node->eval = white_to_move ? std::numeric_limits<double>::lowest() : (std::numeric_limits<double>::max)();

		for (int i = 0; i < node->order_next.size(); i++)
		{
			const auto move_node = node->order_next[i];
			minimax(move_node, pos, alpha, beta, (std::max)(depth - 1, 0));
			double eval = move_node->eval;
			//std::cout << "Evaluating move " << move_to_string(move_node->move) << " Depth " << depth << " Eval is " << eval << "AB [" << alpha << "," << beta << "]\n";

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

			if (beta <= alpha) // cutoff reached - stop looking at other moves
			{
				break;
			}
		}

		if (node->move != MOVE_NONE)
		{
			pos.undo_move(node->move, u);
		}

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

	std::string play_move(Position& pos, double& eval, const int max_depth)
	{
		// Iterative Deepening searches the move tree by iteratively increasing 
		// the depth to a max ddepth. This may sound counterintuitive, since
		// we end up evaluating the same positions again, but the mover ordering matters.
		// After every search at a lower depth, we re-order the move search order
		// so that alpha beta pruning can be more effective

		std::shared_ptr<MoveNode> no_move = std::make_shared<MoveNode>();

		for (int depth = 1; depth <= max_depth; depth++)
		{
			minimax(no_move, pos, std::numeric_limits<double>::lowest(), (std::numeric_limits<double>::max)(), depth);
		}

		//std::ofstream file;
		//file.open("Lines.txt");

		//int lines_seen = 0;
		//print_tree(no_move, "", lines_seen, file);
		//file << "Lines seen " << lines_seen << "\n";

		//file.close();

		for (size_t i = 0; i < no_move->order_next.size(); i++)
		{
			std::cout << "Eval of move [" << move_to_string(no_move->order_next[i]->move) << "] is {" << no_move->order_next[i]->eval << "}\n";
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