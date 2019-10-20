#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "graph_partition.h"
#include "tensor_network.h"

#define tassert(cond,name) {if(!(cond)){std::cout << "TEST FAILED: " name ": " #cond "\n";}}

void test_parsing(){
    std::ifstream file("Samples/4regRand30Node1-p1.qasm");
    std::string str((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
     std::stringstream reader(str);
     std::stringstream writer;
    Circuit c = parseGates(reader);
    printGates(c,writer);
    std::string out_str = writer.str();
    //std::cout << str << "\n\n\n\n\n";
    //std::cout << out_str << "\n\n\n\n\n";
    tassert(str == out_str,"parser_test");
}
void test_graph_partitioning(){
    std::vector<std::vector<size_t>> graph = {
        {1,2},
        {2,0},
        {4},
        {0},
        {3}
    };
    std::vector<size_t> parts = calculate_partitions(graph,2);
    std::cout << "graphpart\n";
    for(size_t p : parts){
        std::cout << p << " ";
    }
    std::cout << "\n";
}
void test_to_tensor_and_to_circuit(){
    std::ifstream file("Samples/catStateEightQubits.qasm");
    Circuit c = parseGates(file);
    TensorNetwork network = from_circuit(c);
    Circuit circ = to_circuit(network);

    std::cout << "original circuit" << "\n";
    printGates(c,std::cout);
    std::cout << "new circuit" << "\n";
    std::cout << "new tensornet size: " << network.tensors.size() << "\n";

    printGates(circ,std::cout);
    std::cout << "circuit test finished" << "\n";
}
void test_multicircuit_decomp(){

    std::ifstream file("Samples/catStateEightQubits.qasm");
    Circuit c = parseGates(file);
    TensorNetwork network = from_circuit(c);
    std::vector<size_t> partition = calculate_partitions(network.forward_edges,2);
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    MultiCircuit multi_circ = to_multi_circuit(multi_network);
    std::cout << "number of subcircuits: " << multi_circ.circuits.size() << "\n";
    printMultiCircuit(multi_circ,std::cout);
}
void test_multigraph_decomp(){
    std::ifstream file("Samples/catStateEightQubits.qasm");
    Circuit c = parseGates(file);
    TensorNetwork network = from_circuit(c);
    std::vector<size_t> partition = calculate_partitions(network.forward_edges,2);
    MultiGraphNetwork multi_network = create_multi_graph_network(network,partition);
    std::cout << "number of subnetworks: " << multi_network.nodes.size() << "\n";
    for(TensorNetwork & net : multi_network.nodes){
        std::cout << "number of nodes in network: " << net.size() << '\n';
        Circuit circ = to_circuit(net);

        printGates(circ,std::cout);
        std::cout << "\n";
    }
}
int main (int narg, char** varg) {
    test_parsing();
    test_graph_partitioning();
    //simulate("Samples/4regRand20Node5-p1.qasm");
    //simulate("Samples/rand-nq6-cn2-d10_rxz.qasm");
    test_to_tensor_and_to_circuit();
    test_multigraph_decomp();
    test_multicircuit_decomp();
}
