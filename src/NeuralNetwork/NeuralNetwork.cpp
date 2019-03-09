/*
** author : Himangshu Saikia, 2017
** email : saikia@kth.se
** adapted from the youtube video by Dave Miller
** https://youtu.be/KkwX7FkLfug
*/

#include "NeuralNetwork.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <fstream>

namespace NN {
	NeuralNetwork::NeuralNetwork()
	{
	}

	void NeuralNetwork::init(const std::vector<size_t>& numNodesPerLayer, const std::vector<double>& eta, const std::vector<double>& alpha, const std::vector<ActivationFunction>& actFuns)
	{
		assert(numNodesPerLayer.size() == eta.size() && eta.size() == alpha.size() && alpha.size() == actFuns.size());

		auto N = numNodesPerLayer.size();

		if (N < 2) {
			std::cout << "Please input an appropriate topology! Two layers minimum!\n";
		}

		layers_.clear();

		for (auto l = 0U; l < numNodesPerLayer.size(); l++) {

			Layer L;

			for (auto n = 0U; n <= numNodesPerLayer[l]; n++) {
				auto numOuts = l == numNodesPerLayer.size() - 1 ? 0 : numNodesPerLayer[l + 1];
				L.push_back(Neuron(n, numOuts, eta[l], alpha[l], actFuns[l]));
			}

			//Force the bias neuron's output to be 1.0
			L.back().setOutputVal(1.0);
			layers_.push_back(L);

		}
	}

	void NeuralNetwork::feedForward(const std::vector<double>& inputVals)
	{
		assert(inputVals.size() == layers_[0].size() - 1);

		//set the input layer
		for (auto i = 0U; i < inputVals.size(); i++) {
			layers_[0][i].setOutputVal(inputVals[i]);
		}

		//feed forward through the other layers
		for (auto l = 1U; l < layers_.size(); l++) {
			for (auto n = 0U; n < layers_[l].size() - 1; n++) {
				layers_[l][n].feedForward(layers_[l - 1]);
			}
		}

	}

	void NeuralNetwork::backProp(const std::vector<double>& outputVals)
	{
		//Calculate overall NET error

		Layer& outputLayer = layers_.back();
		error_ = 0.0;

		for (auto n = 0U; n < outputLayer.size() - 1; n++) {
			double delta = outputVals[n] - outputLayer[n].getOutputVal();
			error_ += delta * delta; // sum of squared errors
		}

		error_ /= outputLayer.size() - 1;
		error_ = sqrt(error_); //RMS

		// Calculate output layer gradients

		for (auto n = 0U; n < outputLayer.size() - 1; n++) {
			outputLayer[n].calculateOutputGradients(outputVals[n]);
		}

		// Calculate hidden layer gradients

		for (auto l = layers_.size() - 2; l > 0; l--) {
			Layer& hiddenLayer = layers_[l];
			Layer& nextLayer = layers_[l + 1];

			for (auto n = 0U; n < hiddenLayer.size(); n++) {
				hiddenLayer[n].calculateHiddenGradients(nextLayer);
			}

		}

		//update connection weights

		for (auto l = layers_.size() - 1; l > 0; l--) {
			Layer& layer = layers_[l];
			Layer& prevLayer = layers_[l - 1];

			for (auto n = 0U; n < layer.size() - 1; n++) {
				layer[n].updateInputWeights(prevLayer);
			}

		}
	}

	void NeuralNetwork::getResults(std::vector<double>& values) const
	{
		values.clear();
		for (auto n = 0U; n < layers_.back().size() - 1; n++) {
			values.push_back(layers_.back()[n].getOutputVal());
		}
	}

	double NeuralNetwork::getError() const
	{
		return error_;
	}

	void NeuralNetwork::readNNFromFile(const std::string & filename)
	{
		std::ifstream file;
		file.open(filename);

		if (file.is_open()) {

			size_t layers;
			file >> layers;
			std::vector<size_t> topology(layers);
			std::vector<double> eta(layers);
			std::vector<double> alpha(layers);
			std::vector<ActivationFunction> actFuns(layers);

			for (auto i = 0U; i < layers; i++) {
				file >> topology[i];
				file >> eta[i];
				file >> alpha[i];

				int actFunInt;

				file >> actFunInt;

				if (actFunInt < int(NUM_OF_FUNS)) {
					actFuns[i] = ActivationFunction(actFunInt);
				}
				else {
					actFuns[i] = RELU;
				}
			}

			init(topology, eta, alpha, actFuns);

			for (auto i = 0U; i < layers - 1; i++) {
				for (auto j = 0U; j <= topology[i]; j++) { // considering bias neuron
					for (auto k = 0U; k < topology[i + 1]; k++) {
						double w;
						double dw;
						file >> w >> dw;
						layers_[i][j].setConnectionWeights(k, w, dw);
					}
				}
			}
		}

		file.close();
	}

	void NeuralNetwork::writeNNToFile(const std::string & filename) const
	{
		std::ofstream file;
		file.open(filename);

		if (file.is_open()) {
			file << layers_.size() << std::endl;
			for (auto i = 0U; i < layers_.size(); i++) {
				file << layers_[i].size() - 1 << " ";
				file << layers_[i][0].getEta() << " ";
				file << layers_[i][0].getAlpha() << " ";
				file << int(layers_[i][0].getActFun()) << std::endl;
			}

			for (auto i = 0U; i < layers_.size() - 1; i++) { // not considering last layer
				for (auto j = 0U; j < layers_[i].size(); j++) { // considering bias neuron
					for (auto k = 0U; k < layers_[i + 1].size() - 1; k++) { // not considering bias neuron
						auto con = layers_[i][j].getConnectionWeight(k);
						file << con.weight << " " << con.deltaWeight << std::endl;
					}
				}
			}
		}

		file.close();
	}
}
