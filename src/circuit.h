#pragma once
#include <vector>
#include "gates.h"

struct GateInfo{
    TensorOpInfo op;
    uint64_t bit1;
    uint64_t bit2;
};
struct Circuit{
    uint64_t num_qubits=0;
    std::vector<GateInfo> gates;
};
