#pragma once
#include "engine.h"
#include <random>

struct Playouts 
{
	size_t moveIdx;
	size_t games;
	size_t whiteWins;
	size_t blackWins;
	double score;
	double winRate;

	Playouts();
	void addResult(int result);
	void setScore(const bool rootPosWhiteToMove, const size_t totalGames);
	static bool comparePlayouts(const Playouts& a, const Playouts& b);
	static bool comparePlayoutsByWinRate(const Playouts& a, const Playouts& b);
};

class MctsEngine : public Engine
{
public:
	MctsEngine();
	std::string playMove(const int positionsToAnalyze, bool debug) override;
private:
	int performPlayout(const std::shared_ptr<Position> pos, const Chess::Move move, int depth);
	std::default_random_engine rng_;
};