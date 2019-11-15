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


std::vector<qcomplex> exact_simulate_circuit(const Circuit & c);
//std::vector<CircuitSample> sampled_simulate_circuit(const Circuit & c);
CircuitSamples sampled_simulate_multicircuit(const MultiCircuit & m);
