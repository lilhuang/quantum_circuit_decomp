#pragma once
#include "gates.h"
#include <vector>

class TensorNetwork{
    std::vector<TensorOpInfo> tensors;
    std::vector<std::vector<size_t>> edges;
};
