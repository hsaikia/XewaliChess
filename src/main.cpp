#include "train.h"
#include <ctime>
using namespace Chess;

int main() {
	//Train 20 games
	int games = 500;

	Train t;
	//t.net_.readNNFromFile("Output/NN.network");
	//t.readBook("Output/Positions.book");
	srand(time(NULL));

	for (int i = 0; i < games; i++) {
		int res = t.playTrainingGame(i + 1);
	}

	getchar();
	return 0;
}