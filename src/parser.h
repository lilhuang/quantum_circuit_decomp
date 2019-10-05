#pragma once
#include <vector>
#include "circuit.h"
#include <istream>
#include <ostream>

Circuit parseGates(std::istream & input);
void printGates(Circuit circuit,std::ostream & output);
