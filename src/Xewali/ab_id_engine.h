/*
* author: Himangshu Saikia, 2018-2021
* email : himangshu.saikia.iitg@gmail.com
*/

#pragma once
#include <string>
#include <vector>
#include "Chess/position.h"

namespace AbIterDeepEngine
{
	/// initializes bitboards
	void init();

	/// Finds the best move at the given position using minimax and iterative deepening
	/// The move tree is pruned using alpha beta pruning
	/// @param[in] pos The position
	/// @param[out] eval The evaluation at the current position
	/// @param[in] max_depth The max depth to search
	/// @return the best move
	std::string play_move(Chess::Position& pos, double& eval, const int max_depth);

	/// Sets a position using a position denoted by a fen string and a sequence of moves following the position
	/// @param[out] pos The position
	/// @param[in] fen The start position as a fen string
	/// @param[in] moves The sequence of moves from the start position
	void set_position(Chess::Position& pos, const std::string & fen, const std::vector<std::string>& moves);
}