#pragma once
#include "gates.h"
#include <vector>
#include <cassert>
#include <unordered_map>
#include "circuit.h"

enum class TensorTy{GATE,TENSOR,EDGE_CUT,CONSTANT,INPUT,OUTPUT,FINAL_OUTPUT};
constexpr size_t NULL_IO_LAB = size_t(-1);

/*class TensorNetwork;
class TensorItem{
    size_t tensor_io_size;
    TensorNetwork * network=nullptr;
    TensorItem(const TensorNetwork & new_net){
        network = new TensorNetwork(new_net);
    }
    TensorItem(const TensorItem & item) = delete;
    ~TensorItem(){
        delete network;
    }
};*/
struct TensorInfo{
    TensorTy type;
    size_t io_label;
    OpInfo op;
};
struct TensorNetwork{
    std::vector<TensorInfo> tensors;
    std::vector<std::vector<size_t>> forward_edges;
    std::vector<std::vector<size_t>> backward_edges;
    void add_node(TensorInfo info){
        tensors.push_back(info);
        forward_edges.push_back();
        backward_edges.push_back();
    }
    void add_edge(size_t src,size_t dest){
        forward_edges.at(src).push_back(dest);
        backward_edges.at(dest).push_back(src);
    }
    size_t size()const{
        return tensors.size();
    }
    template<class fn_ty>
    void iter_edges(size_t node,fn_ty fn){
        for(size_t edge : forward_edges[node]){
            fn(edge);
        }
        for(size_t edge : backward_edges[node]){
            fn(edge);
        }
    }
};
TensorNetwork from_circuit(Circuit circ){
    TensorNetwork network;
    std::vector<size_t> last_used(circ.num_qubits);
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        network.add_node(TensorInfo{.type=CONSTANT,.io_label=bit,.op=NULL_OP});
        last_used[bit] = bit;
    }
    for(const GateInfo & gate : circ.gates){
        assert(gate.bit1 != NULL_BIT);
        size_t cur_idx = tensors.size();
        network.add_node(TensorInfo{.type=GATE,.io_label=NULL_IO_LAB,.op=gate.op});
        if(gate.bit2 == NULL_BIT){
            network.add_edge(last_used.at(gate.bit1),cur_idx);
            last_used.at(gate.bit1) = cur_idx;
        }
        else{
            network.add_edge(last_used.at(gate.bit1),cur_idx);
            network.add_edge(last_used.at(gate.bit2),cur_idx);
            last_used.at(gate.bit1) = cur_idx;
            last_used.at(gate.bit2) = cur_idx;
        }
    }
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        size_t cur_idx = tensors.size();
        network.add_node(TensorInfo{.type=FINAL_OUTPUT,.io_label=bit,.op=NULL_OP});
        network.add_edge(last_used.at(i),cur_idx);
    }
    return network;
}
/*void create_network_seperations(TensorNetwork & network,const std::vector<size_t> & partitioning){
    assert(network.tensors.size() == partitioning.size());
    for(size_t n = 0; n < partitioning.size(); n++){
        for(size_t & e : networks.forward_edges[n]){
            if(partitioning.at(e) != partitioning.at(n)){
                size_t measure_node = tensors.size();
                size_t src_node = n;
                size_t dest_node = e;

                network.add_node(TensorInfo{.type=EDGE_CUT,.io_label=NULL_IO_LAB,.op=NULL_OP});

                e = measure_node;//set forward edge to current node
                network.forward_edges[measure_node].push_back(dest_node);
            }
        }
    }
}*/
//enum class ConectionTy{TRUE_INPUT,};
constexpr size_t NULL_CON = size_t(-1);
struct Connector{
    size_t node=NULL_CON;
    size_t idx=NULL_CON;
};
struct MultiTensorItem{
    TensorNetwork network;
};
struct MultiGraphNetwork{
    std::vector<std::vector<Connector>> forward_edges;
    std::vector<std::vector<Connector>> backward_edges;
    std::vector<TensorNetwork> nodes;
};
MultiGraphNetwork create_multi_graph_network(TensorNetwork network,const std::vector<size_t>& partition_idxs){
    MultiGraphNetwork graph_network;
    size_t old_size = network.tensors.size();
    size_t num_partitions = 1+(*std::max_element(partition_idxs.begin(),partition_idxs.end()));

    std::vector<std::vector<size_t>> partition_nodes(num_partitions);
    for(size_t node = 0; node < old_size; node++){
        partition_nodes[partition_idxs[node]].push_back(node);
    }
    std::vector<TensorNetwork> tensor_networks(num_partitions);
    for(std::vector<size_t> nodes : partition_nodes){
        std::unordered_map<size_t,size_t> node_mapping;
        TensorNetwork new_tn;
        for(size_t node : nodes){
            node_mapping[node] = new_tn.size();
            new_tn.add_node(network.tensors[node]);
            for(size_t edge : network.forward_edges[node]){
                if(partition_idxs[edge] != partition_idxs[node]){
                    new_tn
                }
            }
        }
    }
}
Circuit to_circuit(TensorNetwork network,std::vector<size_t> input_mapping){
    Circuit circ;

}
