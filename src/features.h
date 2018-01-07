/*
* author: Himangshu Saikia, 2018
* email : saikia@kth.se
*/
#pragma once
#include <vector>
#include <memory>
#include "Chess/position.h"
#include "Chess/bitboard.h"
#include "Chess/direction.h"
#include "Chess/mersenne.h"
#include "Chess/movepick.h"
#include "Chess/san.h"


using namespace Chess;

/*
** Important to align all features so that their mean is zero.
*/

class Features {
	
public:
	Features();
	std::vector<double> getFeatureVector() const;
	void setFeaturesFromPos(const std::shared_ptr<Position> pos);
	void printFeatureVector() const;
	static size_t getNumFeatures();

private:
	static const size_t numFeatures_ = 10;
	std::vector<double> V;
};

