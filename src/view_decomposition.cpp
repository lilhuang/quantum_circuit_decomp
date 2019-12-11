#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "graph_partition.h"
#include "tensor_network.h"
#include "genetic_partitioning.h"

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
void out_probabilties(std::ostream & os, CircuitProbs & prob, int num_qubits){
    os << "{\n";
    for(auto probpair : prob){
        os << "\"";
        for(int i =0; i < num_qubits; i++){
            os << int(probpair.first[i]);
        }
        os << "\"";
        os << ": " << probpair.second;
        os << ",";
    }
    os << "}\n";
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
    std::vector<size_t> partition = calculate_best_partition(network.forward_edges,[=](std::vector<size_t> part){
        return compute_cost(max_qubits,part,network);
    });

    /*std::vector<size_t> partition = calculate_best_working_partition(network.forward_edges,[&](std::vector<size_t> part){
        MultiGraphNetwork multi_network = create_multi_graph_network(network,part);
        MultiCircuit multi_circ = to_multi_circuit(multi_network);
        size_t num_qubits = num_qubits_used(multi_circ);
        return num_qubits <= max_qubits;
    });*/
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    std::cout << "#number of subcircuits: " << multi_circ.circuits.size() << "\n";
    printMultiCircuit(multi_circ,std::cout);
    print_communication_cost(multi_circ);
    std::ofstream graphvizfile("graph.vis");
    printPartitioning(network,partition,graphvizfile);
    //CircuitSamples samps = true_samples(c,10000);
    std::vector<double> prob_mags = probability_mags(exact_simulate_circuit(c));
    CircuitProbs exact_probs = mags_to_probs(prob_mags);
    std::ofstream exact("exact.json");
    out_probabilties(exact,exact_probs,c.num_qubits);
    return 1;
    CircuitProbs c2 = sampled_simulate_multicircuit(multi_circ,40);
    std::cout << "similarity_multicirc: " << similarity(c2,prob_mags) << "\n";
    out_probabilties(std::cout,c2,c.num_qubits);
    std::cout << std::endl;
    c2 = sampled_simulate_multicircuit(multi_circ,400);
    std::cout << "similarity_multicirc: " << similarity(c2,prob_mags) << "\n";
    out_probabilties(std::cout,c2,c.num_qubits);
    std::cout << std::endl;
    c2 = sampled_simulate_multicircuit(multi_circ,2304);
    std::cout << "similarity_multicirc: " << similarity(c2,prob_mags) << "\n";
    out_probabilties(std::cout,c2,c.num_qubits);
    std::cout << std::endl;
    c2 = sampled_simulate_multicircuit(multi_circ,40000);
    std::cout << "similarity_multicirc: " << similarity(c2,prob_mags) << "\n";
    std::ofstream out40k("output40k.json");
    out_probabilties(out40k,c2,c.num_qubits);
    std::cout << std::endl;
    c2 = sampled_simulate_multicircuit(multi_circ,400000);
    std::cout << "similarity_multicirc: " << similarity(c2,prob_mags) << "\n";
    std::ofstream out400k("output400k.json");
    out_probabilties(out400k,c2,c.num_qubits);
    std::cout << std::endl;
    //std::cout << "similarity_circ: " << similarity(samps,prob_mags) << "\n";
}
