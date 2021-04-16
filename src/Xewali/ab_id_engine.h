/*
* author: Himangshu Saikia, 2018
* email : himangshu.saikia.iitg@gmail.com
*/

#pragma once
#include <string>
#include <vector>
#include "Chess/position.h"

namespace AbIterDeepEngine
{
	void init();
	std::string play_move(Chess::Position& pos, double& eval, const int max_depth);
	void set_position(Chess::Position& pos, const std::string & fen, const std::vector<std::string>& moves);
}