#pragma once
#include <vector>
#include "circuit.h"
#include "tensor_network.h"
#include <istream>
#include <ostream>

Circuit parseGates(std::istream & input);
void printGates(Circuit circuit,std::ostream & output);
void printMultiCircuit(MultiCircuit multi_circ,std::ostream & output);
void printPartitioning(TensorNetwork tn, std::vector<size_t> partitioning, std::ostream & os);
