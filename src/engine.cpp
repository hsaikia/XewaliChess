/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/

#include "engine.h"
#include "features.h"
#include <algorithm>
#include <sstream>
#include <fstream>

const int trainingGameMovesLimit = 100;

Engine::Engine()
{
	isReady_ = false;
}

void Engine::init()
{
	init_mersenne();
	init_direction_table();
	init_bitboards();
	Position::init_zobrist();
	Position::init_piece_square_tables();
	MovePicker::init_phase_table();

	book_.clear();

	std::vector<size_t> topo = { Features::getNumFeatures(), 3, 1 };
	std::vector<double> eta = { 0.15, 0.15, 0.15 };
	std::vector<double> alpha = { 0.5, 0.5, 0.5 };
	std::vector<ActivationFunction> actFuns = { TANHL, TANHL, TANH };
	net_.init(topo, eta, alpha, actFuns);
	isReady_ = true;
}

bool Engine::isReady() const
{
	return isReady_;
}

void Engine::setPosition(const std::string & fen, const std::vector<std::string>& moves)
{
	isReady_ = false;
	pos_ = std::make_shared<Position>(fen);
	
	for (auto& move : moves) {
		UndoInfo u;
		Position posTemp = *pos_;
		pos_->do_move(move_from_string(posTemp, move), u);
	}
	isReady_ = true;
}

std::string Engine::playMove(const int positionsToAnalyze)
{
	isReady_ = false;
	int result;

	if (Engine::checkTermination(pos_, result, false)) {
		isReady_ = true;
		return "";
	}

	int maxDepth = 0;
	double avgDepth = 0;
	for (int p = 0; p < positionsToAnalyze; p++) {
		int depth;
		searchSubtree(pos_, book_, depth, false);
		maxDepth = max(depth, maxDepth);
		avgDepth = (avgDepth * p + depth) / (p + 1);
	}

	std::cout << "Max Depth searched " << maxDepth << "\n";
	std::cout << "Average Depth " << avgDepth << "\n";

	Move m = pickNextMove(pos_, book_, true, false); // with debug info printed
	isReady_ = true;
	return move_to_string(m);
}

int Engine::playGame(const int id, int movesToCheck, bool trainNN)
{
	auto pos = std::make_shared<Position>(StartPosition);
	int one_side_moves = 0;
	int result; // 1 white, -1 black, 0 draw
	std::vector<Move> game;
	//std::map<std::string, Record> book;

	while (true) { // game continues

		if (Engine::checkTermination(pos, result, one_side_moves >= 2 * trainingGameMovesLimit)) {
			break;
		}

		// Evaluate the subtree under this position using MCTS
		// Evaluate n positions in this subtree

		
		int maxDepth = 0;
		double avgDepth = 0;
		for (int p = 0; p < movesToCheck; p++) {
			int depth;
			searchSubtree(pos, book_, depth, trainNN);
			maxDepth = max(depth, maxDepth);
			avgDepth = (avgDepth * p + depth) / (p + 1);
		}

		std::cout << "Searching done.. picking move " << (one_side_moves + 2) / 2 << "\n";
		std::cout << "Max Depth searched " << maxDepth << "\n";
		std::cout << "Average Depth " << avgDepth << "\n";

		// Now pick the next move
		// Ignore exploration and pick the best average MCTS value move
		//Move m = pickNextMove(pos, book_, true, false); // no debug info printed
		Move m = pickNextMove(pos, book_, true, true); // with debug info printed
		std::string C = pos->side_to_move() == Color::WHITE ? "White" : "Black";

		//play a move
		UndoInfo u;
		game.push_back(m);
		pos->do_move(m, u);
		one_side_moves++;

		auto fen = pos->to_fen();

		std::cout << C <<" played move with MCTS score " << book_[fen].averageEval << "\n";
		std::cout << "Evaluation " << evaluatePosition(pos, trainNN) << "\n";

		getchar();
	}

	//train on the game if result is a win or loss
	if (result != 0 && trainNN) {
		trainGame(game, result);
	}

	std::stringstream ss;
	std::string resStr = "_draw_";
	if (result == 1) {
		resStr = "_white_";
	}
	else if (result == -1) {
		resStr = "_black_";
	}

	ss << "Output/Game" << id
		<< resStr
		<< (one_side_moves + 2) / 2 << "moves"
		<< ".pgn";
	Engine::writeToPgn(game, result, ss.str());
	
	std::cout << "Played a game of " << (game.size() + 1) / 2 << " moves. Result " << result << " \n";
	std::cout << "Book has " << book_.size() << " entries.\n";

	if (trainNN) {

		ss.str(std::string());
		ss << "Output/NN" << id << ".network";
		net_.writeNNToFile(ss.str());

	}
	ss.str(std::string());
	ss << "Output/Positions" << id << ".book";
	writeBook(ss.str());

	return result;
}

void Engine::searchSubtree(const std::shared_ptr<Position> pos, std::map<std::string, Record>& book, int& depth, bool NN)
{
	std::shared_ptr<Position> leaf = std::make_shared<Position>(*pos);
	std::vector<Move> moves;
	int result = 0;
	bool reachedTerminalPosition = false;
	//traverse until leaf is found
	depth = 0;
	while (true) {
		reachedTerminalPosition = Engine::checkTermination(leaf, result, depth >= 2 * trainingGameMovesLimit);
		auto fen = leaf->to_fen();
		if (book.find(fen) == book.end()) { // never seen this position before!
			break;
		}

		if (reachedTerminalPosition) {
			break;
		}

		Move m = pickNextMove(leaf, book, false, false);
		moves.push_back(m);

		UndoInfo u;
		leaf->do_move(m, u);
		depth++;
	}

	// Now we encountered a leaf which has not been seen or evaluated before
	// Evaluate it using the NN
	double val = evaluatePosition(leaf, NN);

	// If the leaf ends up in a win, loss or draw, set the value accordingly
	if (reachedTerminalPosition) {
		val = double(result);
	}

	// Add the leaf to the book
	auto fen = leaf->to_fen();
	book[fen].seen = 1;
	book[fen].averageEval = val;

	// Update the evaluations of all nodes prior to this leaf
	std::shared_ptr<Position> tmp = std::make_shared<Position>(*pos);
	for (int i = 0; i < moves.size(); i++) {
		auto fen = tmp->to_fen();
		book[fen].averageEval = (book[fen].averageEval * book[fen].seen + val) / (book[fen].seen + 1);
		book[fen].seen++;

		UndoInfo u;
		tmp->do_move(moves[i], u);
	}

}

void Engine::backProp(const std::shared_ptr<Position> pos, int result)
{		
	Features f;
	f.setFeaturesFromPos(pos);
	
	auto v = f.getFeatureVector();
	net_.feedForward(v);

	std::vector<double> res;
	res.push_back(result);

	net_.backProp(res);
}

void Engine::trainGame(const std::vector<Move>& game, int result)
{
	std::shared_ptr<Position> pos = std::make_shared<Position>(StartPosition);
	backProp(pos, result);
	for (int i = 0; i < game.size(); i++) {
		UndoInfo u;
		pos->do_move(game[i], u);
		backProp(pos, result);
	}
}

// pick the immediate next move according to current mcts evaluation
// if a next move results in immediate mate, return that move at all costs
Move Engine::pickNextMove(const std::shared_ptr<Position> pos, const std::map<std::string, Record>& book, bool ignoreExploration, bool printDebug)
{
	Move mlist[256];
	bool whiteToMove = (pos->side_to_move() == Color::WHITE);

	auto fen = pos->to_fen(); // for debugging

	auto num_legal_moves = pos->all_legal_moves(mlist);

	if (num_legal_moves == 0) {
		std::cout << "Error! No move possible from position!\n";
		return MOVE_NONE;
	}

	int totPositionsSeenFromThisPos = 0;
	std::vector<Record> mctsVisited(num_legal_moves, Record());

	// find total moves seen from this position
	for (int i = 0; i < num_legal_moves; i++) {
		UndoInfo u;
		std::unique_ptr<Position> posTemp = std::make_unique<Position>(*pos);
		posTemp->reset_game_ply();
		posTemp->do_move(mlist[i], u);

		//found mate
		if (posTemp->is_mate()) {
			return mlist[i];
		}

		auto posFEN = posTemp->to_fen();
		if (book.find(posFEN) != book.end()) { // seen this position before
			auto entry = book.find(posFEN)->second;
			totPositionsSeenFromThisPos += entry.seen;
			mctsVisited[i] = entry;
		}
	}
	
	std::vector<MoveScore> moveScores(num_legal_moves);
	
	for (int i = 0; i < num_legal_moves; i++) {
		moveScores[i].move = mlist[i];
		moveScores[i].index = i;
		
		if (ignoreExploration) {
			moveScores[i].score = mctsVisited[i].averageEval;
		}
		else {
			// if black to move, make probabilities negative
			double score = whiteToMove ? 1 : -1;

			//eval till now + mcts low visit prob

			if (mctsVisited[i].seen == 0) {
				score *= 1000;
			}
			else {
				score *= sqrt(2.0 * log(totPositionsSeenFromThisPos) / (mctsVisited[i].seen));
			}

			// 0.5 to exploration
			// 1 to exploitation

			moveScores[i].score = 0.25 * score + mctsVisited[i].averageEval;
		}
	}

	//sort the moves according to their score
	sort(moveScores.begin(), moveScores.end(), MoveScore::compare);

	if (printDebug) {
		int chosenMoveIdx = whiteToMove ? 0 : num_legal_moves - 1;
		for (int i = 0; i < num_legal_moves; i++) {
			std::cout << "[" << square_to_string(move_from(moveScores[i].move))
				<< square_to_string(move_to(moveScores[i].move))
				<< "] Total Score "
				<< moveScores[i].score
				<< " = Exploitation["
				<< mctsVisited[moveScores[i].index].averageEval
				<< "] + Exploration["
				<< sqrt(2.0 * log(totPositionsSeenFromThisPos + 1) / (mctsVisited[moveScores[i].index].seen + 1))
				<< "] Seen "
				<< mctsVisited[moveScores[i].index].seen;
			if (chosenMoveIdx == i) {
				std::cout << " [Chosen Move]\n";
			}
			else {
				std::cout << "\n";
			}
		}
	}

	// if white to move, choose move with highest score, else choose one with lowest score
	return whiteToMove ? moveScores[0].move : moveScores[num_legal_moves - 1].move;
}

void Engine::writeToPgn(const std::vector<Move>& game, const int result, const std::string & filename)
{
	std::ofstream file;
	file.open(filename);

	auto position = std::make_shared<Position>(StartPosition);
	for (size_t i = 0; i < game.size(); i++) {
		auto pieceType = position->type_of_piece_on(move_from(game[i]));
		auto pieceString = piece_type_to_char(pieceType, true);

		if (pieceString == 'P') {
			pieceString = ' ';
		}

		if (i % 2 == 0) {
			file << (i / 2) + 1 << "." << pieceString << move_to_string(game[i]);
		}
		else {
			file << " " << pieceString << move_to_string(game[i]) << " ";
		}
		UndoInfo u;
		position->do_move(game[i], u);
	}

	if (position->is_mate()) {
		file << "#";
	}

	if (result == 1) {
		file << " 1-0";
	}
	else if (result == -1) {
		file << " 0-1";
	}
	else {
		file << " 0.5-0.5";
	}

	file.close();
}

void Engine::writeBook(const std::string & filename) const
{
	std::ofstream file;
	file.open(filename);

	for (auto& entry : book_) {
		file << entry.second.averageEval << " " << entry.second.seen << " " << entry.first << "\n";
	}

	file.close();
}

bool Engine::checkTermination(const std::shared_ptr<Position> pos, int & result, bool drawCondition)
{
	bool whiteToMove = (pos->side_to_move() == Color::WHITE);

	// 50 moves / threefold repetition / insufficient material to mate -> draw
	if (pos->is_draw() || drawCondition) {
		result = 0;
		return true;
	}

	if (pos->is_mate()) {
		result = whiteToMove ? -1 : 1;
		return true;
	}

	Move mlist[256];
	auto num_legal_moves = pos->all_legal_moves(mlist);

	if (num_legal_moves == 0) {
		// Stalemate
		result = 0;
		return true;
	}

	return false;
}

double Engine::evaluatePosition(const std::shared_ptr<Position> pos, bool NN)
{
	//auto fen = pos->to_fen(); // for debugging only

	Features f;
	f.setFeaturesFromPos(pos);

	if (NN) {
		net_.feedForward(f.getFeatureVector());
		std::vector<double> outputs;
		net_.getResults(outputs);
		return outputs[0];
	}
	return f.evalStatic();
}

