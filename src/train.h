#pragma once
#include <vector>
#include <memory>
#include <map>
#include "Chess/position.h"
#include "Chess/position.h"
#include "Chess/bitboard.h"
#include "Chess/direction.h"
#include "Chess/mersenne.h"
#include "Chess/movepick.h"
#include "Chess/san.h"
#include "../../../NeuralNetworkTest/src/NeuralNetwork.h"

using namespace NN;
using namespace Chess;

struct MoveScore {
	size_t move_idx;
	double score;
	double mctsProb;

	MoveScore() {
		score = 1;
		mctsProb = 1;
	}

	static bool compare(MoveScore ms1, MoveScore ms2) {
		return ms1.score * ms1.mctsProb > ms2.score * ms2.mctsProb;
	}
};

struct Record {
	int wWon;
	int bWon;
	int played;

	Record() {
		wWon = 0;
		bWon = 0;
		played = 0;
	}
};

class Train {
private:
	std::map<std::string, Record> book_;
public:
	Train();
	int playTrainingGame(const int id);
	static void writeToPgn(const std::vector<Move>& game, const int result, const std::string& filename);

	/**
	 * Plays out a game from the given position and returns the result
	 *  Uses a neural net to choose the best move to play
	 **/
	int playout(const std::shared_ptr<Position> pos, int drawAfter = 150);

	/*
	* Picks best move using book(MCTS visit prob) + NN 
	*/
	Move pickBestMove(const std::shared_ptr<Position> pos);
	void trainFromPos(const std::vector<Move>& prevMoves, const std::shared_ptr<Position> pos);
	void backProp(const std::shared_ptr<Position> pos, int result);
	void addToBook(const std::shared_ptr<Position> pos, int result);
	void writeBook(const std::string& filename) const;
	void readBook(const std::string& filename);
	static bool checkTermination(const std::shared_ptr<Position> pos, int& result, bool drawCondition);

	NeuralNetwork net_;
};