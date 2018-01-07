/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/

#include "train.h"
#include "features.h"
#include <ctime>
#include <sstream>
using namespace Chess;

int main() {
	//Train N games
	int games = 10;

	Train t;
	//t.net_.readNNFromFile("Output/NN.network");
	//t.readBook("Output/Positions.book");

	for (int i = 0; i < games; i++) {
		int res = t.playGame(i + 1);
	}

	//testing features

	//std::string fen = "rnbqkbnr/pppp1pp1/8/4p2p/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq -";

	//Features f;
	//auto position = std::make_shared<Position>(fen);
	//f.setFeaturesFromPos(position);
	//
	//f.printFeatureVector();

	getchar();
	return 0;
}