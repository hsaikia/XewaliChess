/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/
#pragma once
#include <vector>
#include <memory>
#include "Chess/position.h"
#include "Chess/bitboard.h"

using namespace Chess;

/**
* Important to align all features so that their mean is zero.
*/

class Features {

public:
	Features();
	static double eval_static(Position& pos);
	static double eval_influence(const Position& pos);

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
	static double eval_static_attack_defense_only(const Position& pos);
	static bool has_game_ended(Position& pos, int& result, bool drawCondition);
private:
	static double eval_static_material_only(const Position& pos);





};

