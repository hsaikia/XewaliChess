#pragma once
#include <vector>
#include <memory>
#include "Chess/position.h"

using namespace Chess;

class Features {
	
public:
	Features();
	std::vector<double> getFeatureVector() const;
	void setFeaturesFromPos(const std::shared_ptr<Position> pos);
	void printFeatureVector() const;
	static size_t getNumFeatures();

	bool whiteToMove;
	int legalMoves;
	// 0 -> White, 1 -> Black
	int nQ[2];
	int nB[2];
	int nR[2];
	int nN[2];
	int nP[2];
	bool castleK[2];
	bool castleQ[2];

private:
	static const size_t numFeatures_ = 16;
};

