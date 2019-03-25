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
//#include "Chess/direction.h"
#include "Chess/mersenne.h"
#include "Chess/movepick.h"
#include "Chess/san.h"
#include "NeuralNetwork/NeuralNetwork.h"

using namespace NN;
using namespace Chess;

struct MoveScore {
	int index; // move index in move list
	Move move;
	double score;

	MoveScore() {
		move = MOVE_NONE;
		score = 0.0;
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

class Engine {
public:
	Engine();
	void init();
	bool isReady() const;
	void setPosition(const std::string & fen, const std::vector<std::string>& moves);
	int playGame(const int id, int movesToCheck, bool trainNN);
	std::string playMove(const int positionsToAnalyze, bool debug);
	static void writeToPgn(const std::vector<Move>& game, const int result, const std::string& filename);
	//void playFromPos(const std::shared_ptr<Position> pos);
	
	//void addToBook(const std::shared_ptr<Position> pos, int result);
	void writeBook(const std::string& filename) const;
	//void readBook(const std::string& filename);
	

	//
	NeuralNetwork net_;
	double evalCurrentPosition() const;
	
private:
	void trainGame(const std::vector<Move>& game, int result);
	/*
	* Picks best next move using book(MCTS visit prob) + Average Evaluation
	*/
	Move pickNextMove(
		const std::shared_ptr<Position> pos, 
		const std::map<std::string, Record>& book, 
		bool ignoreExploration,
		bool printDebug
	);
	void searchSubtree(const std::shared_ptr<Position> pos, std::map<std::string, Record>& book, int& depth, bool NN);
	void backProp(const std::shared_ptr<Position> pos, int result);
	static bool checkTermination(const std::shared_ptr<Position> pos, int& result, bool drawCondition);
	double evaluatePosition(const std::shared_ptr<Position> pos, bool NN);

	// reintroducing for bootstrap..
	std::map<std::string, Record> book_;
	std::shared_ptr<Position> pos_;
	bool isReady_;
};