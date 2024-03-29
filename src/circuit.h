#pragma once
#include <vector>
#include "gates.h"

constexpr uint64_t NULL_BIT = uint64_t(-1);
struct GateInfo{
    OpInfo op=NULL_OP;
    uint64_t bit1=NULL_BIT;
    uint64_t bit2=NULL_BIT;
    uint64_t getBit(int b){
        return b == 0 ? bit1 : bit2;
    }
};
struct Circuit{
    uint64_t num_qubits=0;
    std::vector<GateInfo> gates;
};
constexpr size_t EMPTY_REGISTER = -2;
enum class OutputType{FINAL_OUT,REGISTER_OUT,NULL_OUT=-2};
struct MultiCircuit{
    /*
    Contains the circuits plus the classical steps to correctly simulate the output
    */
    size_t num_classical_registers;
    std::vector<std::vector<size_t>> input_registers;
    std::vector<std::vector<size_t>> output_registers;
    std::vector<std::vector<OutputType>> output_types;
    std::vector<Circuit> circuits;
};


uint64_t num_qubits_used(const MultiCircuit & multi_circ);
size_t communi_size(const MultiCircuit & multi_circ);
