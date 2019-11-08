#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "graph_partition.h"
#include "tensor_network.h"

void print_communication_cost(MultiCircuit multi_circ){
    std::cout << "num regs1: " << multi_circ.num_classical_registers << '\n';
    std::cout << "num regs2: " << multi_circ.output_types.size() << '\n';
    size_t count = 0;
    for(auto & v : multi_circ.output_types){
        std::cout << "reg size: " << v.size() << "\n";
        for(OutputType out : v){
            if(out == OutputType::REGISTER_OUT){
                count += 1;
            }
        }
    }
    std::cout << "communi cost: " << count << '\n';
}
std::vector<uint8_t> split_all_outs(size_t t){
    std::vector<uint8_t> r(t,size_t(0));
    for(int i = 0; i < t/4; i++){
        r[i] = 1;
    }
    return r;
}
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
    TensorNetwork network = from_circuit(c,split_all_outs(c.num_qubits));
    std::vector<size_t> partition = calculate_partitions(network.forward_edges,num_parts);
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    std::cout << "#number of subcircuits: " << multi_circ.circuits.size() << "\n";
    printMultiCircuit(multi_circ,std::cout);
    print_communication_cost(multi_circ);
}
