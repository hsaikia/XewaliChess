/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/

#include "engine.h"
#include "features.h"
#include <ctime>
#include <sstream>
using namespace Chess;

//int main1() {
//	//Train N games
//	int games = 1000;
//	int movesToCheck = 6000;
//
//	Engine t;
//	//t.net_.readNNFromFile("Output/NN.network");
//	//t.readBook("Output/Positions.book");
//
//	for (int i = 0; i < games; i++) {
//		int res = t.playGame(i + 1, movesToCheck, false);
//	}
//
//	////testing features
//
//	//std::string fen = "rnbqkbnr/pppp1pp1/8/4p2p/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq -";
//
//	//Features f;
//	//auto position = std::make_shared<Position>(fen);
//	//f.setFeaturesFromPos(position);
//	//
//	//f.printFeatureVector();
//
//	getchar();
//	return 0;
//}

void tokenize(std::string line, std::vector<std::string>& tokens) {
	tokens.clear();
	std::stringstream ss(line);
	std::string s;

	while (getline(ss, s, ' ')) {
		tokens.push_back(s);
	}
}

int main() {
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
			std::cout << "bestmove " << e.playMove(10000) << std::endl;
		}
		else if (tokens[0] == "quit"){
			break;
		}
		else { // no action
			//nothing to do
		}
	}
	return 0;
}