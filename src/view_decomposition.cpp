#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "graph_partition.h"
#include "tensor_network.h"
#include "clustering.h"
#include "genetic_partitioning.h"

uint64_t num_qubits_used(const MultiCircuit & multi_circ){
    uint64_t max_qbits = 0;
    for(const auto & v : multi_circ.circuits){
        max_qbits = std::max(max_qbits,v.num_qubits);
    }
    return max_qbits;
}
size_t communi_size(const MultiCircuit & multi_circ){
    size_t count = 0;
    for(const auto & v : multi_circ.output_types){
        for(OutputType out : v){
            if(out == OutputType::REGISTER_OUT){
                count += 1;
            }
        }
    }
    return count;
}
void print_communication_cost(MultiCircuit multi_circ){
    std::cout << "num regs1: " << multi_circ.num_classical_registers << '\n';
    std::cout << "num regs2: " << multi_circ.output_types.size() << '\n';
    size_t count = 0;
    for(auto & v : multi_circ.output_types){
        std::cout << "reg size: " << v.size() << "\n";
    }
    std::cout << "communi cost: " << communi_size(multi_circ) << '\n';
    std::cout << "num qubits: " << num_qubits_used(multi_circ) << '\n';
}
std::vector<uint8_t> split_all_outs(size_t t){
    std::vector<uint8_t> r(t,size_t(0));
    for(int i = 0; i < t/4; i++){
        r[i] = 1;
    }
    return r;
}
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
    /*std::vector<size_t> partition = calculate_best_partition(network.forward_edges,[=](std::vector<size_t> part){
        return compute_cost(max_qubits,part,network);
    });*/

    std::vector<size_t> partition = calculate_best_working_partition(network.forward_edges,[&](std::vector<size_t> part){
        MultiGraphNetwork multi_network = create_multi_graph_network(network,part);
        MultiCircuit multi_circ = to_multi_circuit(multi_network);
        size_t num_qubits = num_qubits_used(multi_circ);
        return num_qubits <= max_qubits;
    });
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    std::cout << "#number of subcircuits: " << multi_circ.circuits.size() << "\n";
    printMultiCircuit(multi_circ,std::cout);
    print_communication_cost(multi_circ);
    std::ofstream graphvizfile("graph.vis");
    printPartitioning(network,partition,graphvizfile);

    NodeTable n = circuitToNodeTable(c);
    std::vector<size_t> greedyPartition = n.getGreedyClusteredNetworkVector(max_qubits,c);
    std::ofstream greedygraphvizfile("greedygraph.vis");
    printPartitioning(network,greedyPartition,greedygraphvizfile);
}
