/*
** author : Himangshu Saikia
** email : saikia@kth.se
** adapted from the youtube video by Dave Miller
** https://youtu.be/KkwX7FkLfug
*/

#include "Neuron.h"
#include <algorithm>

namespace NN {

	Neuron::Neuron(const size_t idx, const size_t numOutputs, const double eta, const double alpha, const ActivationFunction actFun)
	{
		eta_ = eta;
		alpha_ = alpha;
		actFun_ = actFun;
		idx_ = idx;
		outputWeights_.clear();
		outputWeights_.resize(numOutputs);
	}

	double Neuron::getOutputVal() const
	{
		return outputVal_;
	}

	double Neuron::getEta() const
	{
		return eta_;
	}

	double Neuron::getAlpha() const
	{
		return alpha_;
	}

	ActivationFunction Neuron::getActFun() const
	{
		return actFun_;
	}

	Connection Neuron::getConnectionWeight(const size_t idx) const
	{
		return outputWeights_[idx];
	}

	void Neuron::setOutputVal(double val)
	{
		outputVal_ = val;
	}

	void Neuron::feedForward(const Layer & previousLayer)
	{
		double sum = 0.0;

		for (auto n = 0U; n < previousLayer.size(); n++) {
			sum += previousLayer[n].getOutputVal() *
				previousLayer[n].outputWeights_[idx_].weight;
		}

		outputVal_ = Neuron::transferFunction(sum);

	}

	void Neuron::calculateOutputGradients(double outVal)
	{
		double delta = outVal - outputVal_;
		gradient_ = delta * Neuron::transferFunctionDerivative(outputVal_);
	}

	void Neuron::calculateHiddenGradients(const Layer & nextLayer)
	{
		double dow = sumDOW(nextLayer);
		gradient_ = dow * Neuron::transferFunctionDerivative(outputVal_);
	}

	void Neuron::updateInputWeights(Layer & prevLayer)
	{
		// The weights that are modified are in the Connections in the neurons
		// of the previous layer

		for (auto n = 0U; n < prevLayer.size(); n++) {
			Neuron& N = prevLayer[n];

			double oldDeltaWeight = N.outputWeights_[idx_].deltaWeight;

			double newDeltaWeight =
				(1 - alpha_) * eta_ * N.getOutputVal() * gradient_ + alpha_ * oldDeltaWeight;

			N.outputWeights_[idx_].deltaWeight = newDeltaWeight;
			N.outputWeights_[idx_].weight += newDeltaWeight;
		}
	}

	double Neuron::transferFunction(double x)
	{
		switch (actFun_) {
		case SIGMOID: return 1.0 / (1 + exp(-x)); // output range [0, 1]
		case TANH: return tanh(x); // tanh - output range [-1, 1]
		case TANHL: return 1.7 * tanh(2 * x / 3) + 0.05 * x; // tanh - output range [-1, 1]
		case RELU: return std::max(0.0, x); // output range [0, inf]
		case LEAKY_RELU: return x > 0.0 ? x : 0.01 * x; // output range [-inf, inf]
		case ELU: return x > 0.0 ? x : ELU_alpha * (exp(x) - 1); // output range [-ELU_alpha, inf]
		default: break;
		}

		return 0.0;
	}

	double Neuron::transferFunctionDerivative(double x)
	{

		switch (actFun_) {
		case SIGMOID: return exp(-x) / pow(1 + exp(-x), 2); // output range [0, 1]
		case TANH: return 1.0 - pow(tanh(x), 2); // tanh - output range [-1, 1]
		case TANHL: return 1.7 * (1.0 - pow(tanh(2 * x / 3), 2)) + 0.05;
		case RELU: return x > 0.0 ? 1 : 0; // output range [0, inf]
		case LEAKY_RELU: return x > 0.0 ? 1 : 0.01; // output range [-inf, inf]
		case ELU: return x > 0.0 ? 1 : ELU_alpha * exp(x); // output range [-ELU_alpha, inf]
		default: break;
		}

		return 0.0;
	}

	void Neuron::setConnectionWeights(const size_t idx, const double w_, const double dw_)
	{
		outputWeights_[idx].weight = w_;
		outputWeights_[idx].deltaWeight = dw_;
	}

	double Neuron::sumDOW(const Layer & nextLayer)
	{
		double sum = 0.0;

		for (auto n = 0U; n < nextLayer.size() - 1; n++) {
			sum += outputWeights_[n].weight * nextLayer[n].gradient_;
		}

		return sum;
	}
}