#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <complex>
#include "tensor_network.h"
#include "circuit.h"

enum class SampleTy:uint8_t{I,X,Y,Z};
using qcomplex = std::complex<double>;

class CircuitSample{
private:
    std::vector<SampleTy> samples;
    std::vector<uint8_t> sample_bits;
public:
    CircuitSample(int num_bits):
        sample_bits(num_bits){}
    void set_bit(int idx,bool value){sample_bits.at(idx) = !!value;}
    bool get_bit(int idx,bool value){return sample_bits.at(idx);}
};
struct SampleState{
    std::vector<CircuitSample> samples;
};

std::vector<qcomplex> exact_simulate_circuit(const Circuit & c);
//std::vector<CircuitSample> sampled_simulate_circuit(const Circuit & c);
std::vector<CircuitSample> sampled_simulate_multicircuit(const MultiCircuit & m,std::vector<size_t> function_decomp);
