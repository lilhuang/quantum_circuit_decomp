#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <complex>
#include <bitset>
#include "tensor_network.h"
#include "circuit.h"

enum class SampleTy:uint8_t{I,X,Y,Z};
using qcomplex = std::complex<double>;

using QuantumFinalOut = std::bitset<256>;

using CircuitSamples = std::unordered_map<QuantumFinalOut,int>;
using CircuitProbs = std::unordered_map<QuantumFinalOut,double>;


std::vector<qcomplex> exact_simulate_circuit(const Circuit & c);
//std::vector<CircuitSample> sampled_simulate_circuit(const Circuit & c);
CircuitSamples true_samples(const Circuit & circuit,int num_samples);
CircuitProbs sampled_simulate_multicircuit(const MultiCircuit & m, size_t number_samples);
double similarity(const CircuitSamples & c1,const CircuitSamples & c2);
double similarity(const CircuitProbs & c1,const std::vector<double> & c2);
std::vector<double> probability_mags(const std::vector<qcomplex> & exact_result);
CircuitProbs mags_to_probs(std::vector<double> & circuit_mags);
