#include "features.h"

Features::Features()
{
}

std::vector<double> Features::getFeatureVector() const
{
	std::vector<double> ret(numFeatures_);
	int index = 0;
	ret[index++] = double(whiteToMove);
	ret[index++] = double(legalMoves);

	for (int i = 0; i < 2; i++) {
		ret[index++] = nQ[i];
		ret[index++] = nB[i];
		ret[index++] = nR[i];
		ret[index++] = nN[i];
		ret[index++] = nP[i];
		ret[index++] = castleK[i];
		ret[index++] = castleQ[i];
	}
	return ret;
}

void Features::setFeaturesFromPos(const std::shared_ptr<Position> pos)
{
	auto side = pos->side_to_move();
	whiteToMove = side == Color::WHITE ? true : false;
	Move mlist[256];
	legalMoves = pos->all_legal_moves(mlist);

	for (int i = 0; i < 2; i++) {
		Color c = Color::BLACK;
		if (i == 0) {
			c = Color::WHITE;
		}
		nQ[i] = pos->queen_count(c);
		nB[i] = pos->bishop_count(c);
		nR[i] = pos->rook_count(c);
		nN[i] = pos->knight_count(c);
		nP[i] = pos->pawn_count(c);
		castleK[i] = pos->can_castle_kingside(c);
		castleQ[i] = pos->can_castle_queenside(c);
	}
}

void Features::printFeatureVector() const
{
	auto v = getFeatureVector();
	std::cout << "[";
	for (int i = 0; i < v.size(); i++) {
		std::cout << v[i] << " ";
	}
	std::cout << "]\n";
}

size_t Features::getNumFeatures()
{
	return numFeatures_;
}
