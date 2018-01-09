/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/

#pragma once
#include <vector>
#include <memory>
#include <map>
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
	Move move;
	double score;

	MoveScore() {
		move = MOVE_NONE;
		score = 1;
	}

	static bool compare(MoveScore ms1, MoveScore ms2) {
		return ms1.score > ms2.score;
	}
};

struct Record {
	double averageEval;
	int seen;

	Record() {
		averageEval = 0.0;
		seen = 0;
	}
};

class Train {
public:
	Train();
	int playGame(const int id, int movesToCheck);
	static void writeToPgn(const std::vector<Move>& game, const int result, const std::string& filename);
	//void playFromPos(const std::shared_ptr<Position> pos);
	
	//void addToBook(const std::shared_ptr<Position> pos, int result);
	void writeBook(const std::string& filename) const;
	//void readBook(const std::string& filename);
	

	//
	NeuralNetwork net_;
	
private:
	/*
	* Picks best next move using book(MCTS visit prob) + Average Evaluation
	*/
	void trainGame(const std::vector<Move>& game, int result);
	Move pickNextMove(const std::shared_ptr<Position> pos, const std::map<std::string, Record>& book);
	void searchSubtree(const std::shared_ptr<Position> pos, std::map<std::string, Record>& book, int& depth);
	void backProp(const std::shared_ptr<Position> pos, int result);
	static bool checkTermination(const std::shared_ptr<Position> pos, int& result, bool drawCondition);
	double evaluatePosition(const std::shared_ptr<Position> pos);

	// reintroducing for bootstrap..
	std::map<std::string, Record> book_;
};