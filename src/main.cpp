/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/

#include "engine.h"
#include "features.h"
#include <ctime>
#include <sstream>
#include <iostream>
//using namespace Chess;

const int positionsToEvaluatePerMove = 100000;
bool debug = true;

int test_main() {
	//Train N games
	int games = 1;

	Engine t;
	t.init();

	////t.net_.readNNFromFile("Output/NN.network");
	////t.readBook("Output/Positions.book");

	for (int i = 0; i < games; i++) {
		int res = t.playGame(i + 1, positionsToEvaluatePerMove, false);
	}

	//testing features
	//std::string fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"; // 1. e4 e5 2. Nf3
	//std::string fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/7N/PPPP1PPP/RNBQKB1R b KQkq - 1 2";  // 1. e4 e5 2. Nh3

	//std::string fen = "7r/p3k3/2Q2p2/1p2b3/4P3/8/PPP2PBP/1K1R3R b - -2 25";

	//std::string fen1 = "rnbqkb1r/ppppp1P1/5n2/7p/8/8/PPPP1PPP/RNBQKBNR w KQkq - 1 5"; // white pawn on 7th rank

	//printf("testing promotions\n");
	//auto pos = std::make_shared<Position>(fen1);
	//Move mlist[256];
	//auto num_legal_moves = pos->all_legal_moves(mlist);

	//printf("Num legal moves %d\n", num_legal_moves);
	//for (int i = 0; i < num_legal_moves; i++) {
	//	printf("%s\n", move_to_string(mlist[i]).c_str());
	//}
	
	//auto position = std::make_shared<Position>(fen);
	//auto eval = Features::evalStatic(position, true);

	//printf("\nFinal Score %.2lf\n", eval);

	// f.setFeaturesFromPos(position);
	// f.printFeatureVector();
	
	getchar();
	return 0;
}

void tokenize(std::string line, std::vector<std::string>& tokens) {
	tokens.clear();
	std::stringstream ss(line);
	std::string s;

	while (getline(ss, s, ' ')) {
		tokens.push_back(s);
	}
}

int ucimain() {
	Engine e;
	std::string line;
	while (getline(std::cin, line)) {
		std::vector<std::string> tokens;
		tokenize(line, tokens);

		if (tokens.size() == 0) {
			continue;
		}

		if (tokens[0] == "uci") {
			std::cout << "id name Xewali 1.0" << std::endl;
			std::cout << "id author Himangshu Saikia" << std::endl;
			std::cout << "uciok" << std::endl;
		}
		else if (tokens[0] == "ucinewgame") {
			e.init();
		}
		else if (tokens[0] == "isready") {
			std::cout << "readyok" << std::endl;
		}
		else if (tokens[0] == "position") {
			std::string fen = "";
			std::vector<std::string> moves;
			bool readingFEN = true;

			if (tokens[1] == "startpos") {
				fen = StartPosition;
			}
			else if (tokens[1] == "fen") {
				// do nothing
			}

			for (int i = 2; i < tokens.size(); i++) {
				if (tokens[i] == "moves") {
					readingFEN = false;
					continue;
				}
				if (readingFEN) {
					fen += tokens[i];
					fen += " ";
				}
				else {
					moves.push_back(tokens[i]);
				}
			}
			e.setPosition(fen, moves);
		}
		else if (tokens[0] == "go") {
			std::cout << "info Thinking..." << std::endl;
			std::cout << "bestmove " << e.playMove(positionsToEvaluatePerMove, debug) << std::endl;
		}
		else if (tokens[0] == "quit"){
			break;
		}
		else if (tokens[0] == "eval") {
			std::cout << e.evalCurrentPosition() << std::endl;
		}
		else { // no action
			//nothing to do
		}
	}
	return 0;
}

int main() {
	ucimain();
	//test_main();
	return 0;
}
