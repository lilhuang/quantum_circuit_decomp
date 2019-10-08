#pragma once
#include <vector>
#include "gates.h"

constexpr uint64_t NULL_BIT = uint64_t(-1);
struct GateInfo{
    OpInfo op=NULL_OP;
    uint64_t bit1=NULL_BIT;
    uint64_t bit2=NULL_BIT;
};
struct Circuit{
    uint64_t num_qubits=0;
    std::vector<GateInfo> gates;
};
struct MultiCircuit{
    /*
    Contains the circuits plus the classical steps to correctly simulate the output
    */
};
