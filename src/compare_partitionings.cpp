#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include "graph_partition.h"
#include "tensor_network.h"
#include "genetic_partitioning.h"

double compute_cost(size_t max_qubits,const std::vector<size_t> & partition,const TensorNetwork & network){
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    size_t com_size = communi_size(multi_circ);
    size_t num_qubits = num_qubits_used(multi_circ);
    double qubit_cost = num_qubits > max_qubits ? -1000000 : -0.1*num_qubits;
    double communi_cost = -10.0*com_size;
    if(num_qubits == 0){
        std::cout << multi_network.nodes.size() << " ";
        std::cout << multi_circ.circuits.size() << " \n";
    }
    return qubit_cost + communi_cost;
}
int num_qubits_partition(const std::vector<size_t> & partition,const TensorNetwork & network){
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    size_t num_commui = communi_size(multi_circ);
    return num_commui;
}
int main (int narg, char** varg) {
    if(narg != 3){
        throw std::runtime_error("need to pass in 2 argument, the number of qubits of the machine and the name of the .qasm file to run.");
    }
    int max_qubits = stoi(std::string(varg[1]));
    std::string fname(varg[2]);

    std::ifstream file(fname);
    if(!file){
        throw std::runtime_error("filename: "+fname+"  could not be read.");
    }
    Circuit c = parseGates(file);
    TensorNetwork network = from_circuit(c);
    int start_gen = clock();
    std::vector<size_t> gentic_partition = calculate_best_partition(network.forward_edges,[=](std::vector<size_t> part){
        return compute_cost(max_qubits,part,network);
    });
    int genetic_time = clock() - start_gen;
    int start_metis = clock();
    std::vector<size_t> metis_partition = calculate_best_working_partition(network.forward_edges,[&](std::vector<size_t> part){
        MultiGraphNetwork multi_network = create_multi_graph_network(network,part);
        MultiCircuit multi_circ = to_multi_circuit(multi_network);
        size_t num_qubits = num_qubits_used(multi_circ);
        return num_qubits <= max_qubits;
    });
    int metis_time = clock() - start_metis;
    std::cout << num_qubits_partition(gentic_partition,network) << "," <<  genetic_time/double(CLOCKS_PER_SEC) << "," <<
            num_qubits_partition(metis_partition,network)  << "," <<  metis_time/double(CLOCKS_PER_SEC) << "\n";

            std::ofstream graphvizfile("graph.vis");
    //printPartitioning(network,metis_partition,graphvizfile);
}
