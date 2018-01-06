#include "train.h"
#include "features.h"
#include <algorithm>
#include <sstream>
#include <fstream>

const int trainingGameMovesLimit = 100;
const double epsilon = 0.2; // for e-greedy strategy
const int numGamesToTrainOn = 50;

Train::Train()
{
	init_mersenne();
	init_direction_table();
	init_bitboards();
	Position::init_zobrist();
	Position::init_piece_square_tables();
	MovePicker::init_phase_table();

	book_.clear();

	std::vector<size_t> topo = { Features::getNumFeatures(), Features::getNumFeatures(), 1 };
	std::vector<double> eta = { 0.15, 0.15, 0.15 };
	std::vector<double> alpha = { 0.2, 0.2, 0.2 };
	std::vector<ActivationFunction> actFuns = { TANH, TANH, TANH };
	net_.init(topo, eta, alpha, actFuns);
}

int Train::playTrainingGame(const int id)
{
	auto pos = std::make_shared<Position>(StartPosition);
	int one_side_moves = 0;
	int result; // 1 white, -1 black, 0 draw
	std::vector<Move> game;

	while (true) { // game continues

		//std::cout << "Playing Game " << id << "\n";
		//std::cout << "Move " << (one_side_moves + 2) / 2 << "\n";

		if (Train::checkTermination(pos, result, one_side_moves >= 2 * trainingGameMovesLimit)) {
			break;
		}

		//train n games
		int n = numGamesToTrainOn;
		while (n--) {
			trainFromPos(game, pos);
		}

		//play a move
		UndoInfo u;
		Move m = pickBestMove(pos);
		game.push_back(m);
		pos->do_move(m, u);
		one_side_moves++;
	}

	std::stringstream ss;
	std::string resStr = "_draw_";
	if (result == 1) {
		resStr = "_white_";
	}
	else if (result == -1) {
		resStr = "_black_";
	}
	ss << "Output/TrainingGame" << id
		<< resStr
		<< (one_side_moves + 2) / 2 << "moves"
		<< ".pgn";
	Train::writeToPgn(game, result, ss.str());

	ss.str(std::string());
	ss << "Output/NN" << id << ".network";
	net_.writeNNToFile(ss.str());

	ss.str(std::string());
	ss << "Output/Positions" << id << ".book";
	writeBook(ss.str());

	ss.str(std::string());
	ss << "Output/Info" << id << ".txt";
	std::ofstream infoFile;
	infoFile.open(ss.str());

	infoFile << "Played a training game of " << (game.size() + 1) / 2 << " moves. Result " << result << " \n";
	infoFile << "Book has " << book_.size() << " entries\n";

	infoFile.close();

	return result;
}

void Train::trainFromPos(const std::vector<Move>& prevMoves, const std::shared_ptr<Position> pos_)
{
	std::shared_ptr<Position> leaf = std::make_shared<Position>(*pos_);
	std::vector<Move> moves = prevMoves;
	int result = 0;
	//traverse until leaf is found
	while (true) {
		auto fen = leaf->to_fen();
		if (book_.find(fen) == book_.end()) { // never seen this position before!
			break;
		}

		if (Train::checkTermination(leaf, result, false)) {
			break;
		}

		Move m;
		double R = rand() / double(RAND_MAX);
		
		// follow e-greedy strategy
		if (R < epsilon) {
			Move mlist[256];
			auto num_legal_moves = leaf->all_legal_moves(mlist);
			int rand_move_idx = rand() % num_legal_moves;
			m = mlist[rand_move_idx];
		}
		else {
			m = this->pickBestMove(leaf);
		}
		UndoInfo u;
		moves.push_back(m);
		leaf->do_move(m, u);
	}

	//perform playout till the end
	result = playout(leaf);

	std::shared_ptr<Position> root = std::make_shared<Position>(StartPosition);

	// if result is won or loss, update NN weights using backprop
	// also update book
	for (int i = 0; i < moves.size(); i++) {
		//nn
		backProp(root, result);

		//book
		addToBook(root, result);

		UndoInfo u;
		root->do_move(moves[i], u);
	}

	// finally, add the leaf node to the book
	backProp(leaf, result);
	addToBook(leaf, result);
}

void Train::backProp(const std::shared_ptr<Position> pos, int result)
{
	// do not train on drawn games!
	if (result != 0) {

		Features f;
		f.setFeaturesFromPos(pos);

		auto v = f.getFeatureVector();
		net_.feedForward(v);

		std::vector<double> res;
		res.push_back(result);

		net_.backProp(res);
	}
}

void Train::addToBook(const std::shared_ptr<Position> pos, int result)
{
	auto fen = pos->to_fen();
	book_[fen].played++;

	if (result == 1) {
		book_[fen].wWon++;
	}
	else if (result == -1) {
		book_[fen].bWon++;
	}
}

int Train::playout(const std::shared_ptr<Position> pos_, int drawAfter)
{
	int movesPlayed = 0;
	int result = 0;

	std::shared_ptr<Position> pos = std::make_shared<Position>(*pos_);

	while (true) { // game continues

		if (checkTermination(pos, result, movesPlayed > 2 * drawAfter)) {
			break;
		}

		UndoInfo u;
		pos->do_move(pickBestMove(pos), u);
		movesPlayed++;
	}

	return result;
}

Move Train::pickBestMove(const std::shared_ptr<Position> pos)
{
	Move mlist[256];
	bool whiteToMove = (pos->side_to_move() == Color::WHITE);

	auto num_legal_moves = pos->all_legal_moves(mlist);

	if (num_legal_moves == 0) {
		std::cout << "Error! No move possible from position!\n";
		return MOVE_NONE;
	}

	int totGamesPlayedFromThisPos = 0;
	std::vector<Record> mctsVisited(num_legal_moves, Record());

	for (int i = 0; i < num_legal_moves; i++) {
		UndoInfo u;
		std::shared_ptr<Position> posTemp = std::make_shared<Position>(*pos);
		posTemp->do_move(mlist[i], u);
		auto posFEN = posTemp->to_fen();
		if (book_.find(posFEN) != book_.end()) { // seen this position before
			totGamesPlayedFromThisPos += book_[posFEN].played;
			mctsVisited[i] = book_[posFEN];
		}
	}
	
	std::vector<MoveScore> moveScores(num_legal_moves);
	
	for (int i = 0; i < num_legal_moves; i++) {
		UndoInfo u;
		moveScores[i].move_idx = i;

		// NN output
		std::shared_ptr<Position> posTemp = std::make_shared<Position>(*pos);
		posTemp->do_move(mlist[i], u);

		Features f;
		f.setFeaturesFromPos(posTemp);

		net_.feedForward(f.getFeatureVector());

		std::vector<double> scores;
		net_.getResults(scores);

		moveScores[i].score = scores[0];

		//mcts
		if (totGamesPlayedFromThisPos > 0) {
			auto Won = whiteToMove ? mctsVisited[i].wWon : mctsVisited[i].bWon;
			moveScores[i].mctsProb = mctsVisited[i].played == 0 ? 1000 :
			double(Won) / mctsVisited[i].played +
			sqrt(2.0 * log(totGamesPlayedFromThisPos) / mctsVisited[i].played);
		}
	}

	// if white to move, choose move with highest score, else choose one with lowest score
	sort(moveScores.begin(), moveScores.end(), MoveScore::compare);

	return whiteToMove ? mlist[moveScores[0].move_idx] : mlist[moveScores[num_legal_moves - 1].move_idx];
}

void Train::writeToPgn(const std::vector<Move>& game, const int result, const std::string & filename)
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

void Train::writeBook(const std::string & filename) const
{
	std::ofstream file;
	file.open(filename);

	for (auto& entry : book_) {
		file << entry.first << "\n" << entry.second.wWon 
			<< " " << entry.second.bWon << " " << entry.second.played << "\n";
	}

	file.close();
}

void Train::readBook(const std::string & filename)
{
	// doesn't clear existing book_, just adds to it
	std::ifstream file;
	file.open(filename);

	std::string line, fen;
	int w, b, p;

	bool key = true;

	while (std::getline(file, line)) {
		if (key) {
			fen = line;
			key = false;
		}
		else {
			std::istringstream is(line);
			is >> w >> b >> p;
			if (book_.find(fen) != book_.end()) {
				book_[fen].wWon += w;
				book_[fen].bWon += b;
				book_[fen].played += p;
			}
			else {
				book_[fen].wWon = w;
				book_[fen].bWon = b;
				book_[fen].played = p;
			}
			key = true;
		}
	}

	file.close();

	std::cout << "Read " << book_.size() << " book entries\n";

}

bool Train::checkTermination(const std::shared_ptr<Position> pos, int & result, bool drawCondition)
{
	bool whiteToMove = (pos->side_to_move() == Color::WHITE);

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
		//std::cout << "Game " << id << " ended in Stalemate.\n";
		result = 0;
		return true;
	}

	return false;
}
