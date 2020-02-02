#pragma once
#include "engine.h"

class AlphaBetaEngine : public Engine
{
public:
	AlphaBetaEngine();
	std::string playMove(const int depth, bool debug) override;
private:
	double minimax(Position& pos, const int depth, double alpha, double beta);
};