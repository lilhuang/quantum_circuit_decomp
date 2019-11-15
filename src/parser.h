#pragma once
#include <vector>
#include "circuit.h"
#include <istream>
#include <ostream>

Circuit parseGates(std::istream & input);
void printGates(Circuit circuit,std::ostream & output);
void printMultiCircuit(MultiCircuit multi_circ,std::ostream & output);
void print_partition_tikz(Circuit circ,std::vector<size_t> partition, std::ostream & os);
