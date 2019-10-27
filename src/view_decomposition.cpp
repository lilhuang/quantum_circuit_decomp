#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "graph_partition.h"
#include "tensor_network.h"

int main (int narg, char** varg) {
    if(narg != 3){
        throw std::runtime_error("need to pass in 2 argument, the number of partitions and the name of the .qasm file to run.");
    }
    int num_parts = stoi(std::string(varg[1]));
    std::string fname(varg[2]);

    std::ifstream file(fname);
    if(!file){
        throw std::runtime_error("filename: "+fname+"  could not be read.");
    }
    Circuit c = parseGates(file);
    TensorNetwork network = from_circuit(c);
    std::vector<size_t> partition = calculate_partitions(network.forward_edges,num_parts);
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    std::cout << "number of subcircuits: " << multi_circ.circuits.size() << "\n";
    printMultiCircuit(multi_circ,std::cout);
}
