/*
* author: Himangshu Saikia, 2018-2021
* email : himangshu.saikia.iitg@gmail.com
*/
#pragma once
#include <vector>
#include <memory>
#include "Chess/position.h"
#include "Chess/bitboard.h"

using namespace Chess;

namespace Features
{
	/// Piece values
	constexpr double KING_VALUE = 2000.0;
	constexpr double QUEEN_VAL = 9.0;
	constexpr double ROOK_VAL = 5.0;
	constexpr double BISHOP_VAL = 3.0;
	constexpr double KNIGHT_VAL = 3.0;
	constexpr double PAWN_VAL = 1.0;
	constexpr double TOT_VAL = (QUEEN_VAL + 2 * ROOK_VAL + 2 * BISHOP_VAL + 2 * KNIGHT_VAL + 8 * PAWN_VAL);

	/// static evaluates the position
	/// @param[in] pos The position
	/// return The evaluation
	double eval_static(Position& pos);

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

	/// Evaluates the position using influence of pieces
	/// @param[in] pos The position
	/// return The evaluation
	double eval_static_attack_defense(const Position& pos);

	/// Checks if the game has ended in a win/loss or draw
	/// @param[in] pos The position
	/// @param[out] result the result of the game (1, -1, 0) if the game has ended
	/// return true if the game has ended, false otherwise
	bool has_game_ended(Position& pos, int& result);

	/// Evaluates the position using material
	/// @param[in] pos The position
	/// return The evaluation
	double eval_static_material(const Position& pos);
};

