/*
** author : Himangshu Saikia, 2017
** email : saikia@kth.se
** adapted from the youtube video by Dave Miller
** https://youtu.be/KkwX7FkLfug
*/

#pragma once 
#include <vector>
#include <cstdlib>
#include "Neuron.h"

namespace NN {
class NeuralNetwork {
public:
	NeuralNetwork();
	void init(
		const std::vector<size_t>& numNodesPerLayer,
		const std::vector<double>& eta,
		const std::vector<double>& alpha,
		const std::vector<ActivationFunction>& actFuns
	);
	void feedForward(const std::vector<double>& inputVals);
	void backProp(const std::vector<double>& outputVals);
	void getResults(std::vector<double>& values) const;
	double getError() const;
	void readNNFromFile(const std::string& filename);
	void writeNNToFile(const std::string& filename) const;
private:
	std::vector<Layer> layers_;
	double error_;
};
}