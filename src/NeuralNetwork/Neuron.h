/*
** author : Himangshu Saikia, 2017
** email : saikia@kth.se
** adapted from the youtube video by Dave Miller
** https://youtu.be/KkwX7FkLfug
*/

#pragma once
#include <vector>
#include <cstdlib>

namespace NN {

	const double ELU_alpha = 0.1; // must be greater than 0
	enum ActivationFunction { SIGMOID, TANH, TANHL, RELU, LEAKY_RELU, ELU, NUM_OF_FUNS };

	struct Connection {

		Connection() {
			//weight = 2.0 * rand() / double(RAND_MAX) - 1.0; // in [-1, 1]
			weight = rand() / double(RAND_MAX); // in [0, 1]
			deltaWeight = 0.0;
		}

		double weight;
		double deltaWeight;
	};

	class Neuron;

	typedef std::vector<Neuron> Layer;

	class Neuron {
	public:
		Neuron(const size_t idx, const size_t numOutputs, const double eta, const double alpha, const ActivationFunction actFun);

		void feedForward(const Layer& previousLayer);
		void calculateOutputGradients(double outVal);
		void calculateHiddenGradients(const Layer& nextLayer);
		void updateInputWeights(Layer& prevLayer);
		double transferFunction(double x);
		double transferFunctionDerivative(double x);

		//setters
		void setOutputVal(double val);
		void setConnectionWeights(const size_t idx, const double w_, const double dw_);

		//getters
		double getOutputVal() const;
		double getEta() const;
		double getAlpha() const;
		ActivationFunction getActFun() const;
		Connection getConnectionWeight(const size_t idx) const;
	private:
		double sumDOW(const Layer& nextLayer);
		double outputVal_;
		double gradient_;
		std::vector<Connection> outputWeights_;
		size_t idx_;
		double eta_; // learning rate
		double alpha_; // momentum
		ActivationFunction actFun_;
	};

}