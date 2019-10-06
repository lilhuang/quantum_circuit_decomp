#pragma once
#include "gates.h"
#include <vector>
#include <cassert>
#include "circuit.h"

enum class TensorTy{GATE,TENSOR,MEASUREMENT,INPUT,OUTPUT};
constexpr size_t NULL_IO_LAB = size_t(-1);

class TensorNetwork;
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
};
struct TensorInfo{
    TensorTy type;
    size_t io_label;
    OpInfo op;
};
struct TensorNetwork{
    std::vector<TensorInfo> tensors;
    std::vector<std::vector<size_t>> forward_edges;
};
TensorNetwork from_circuit(Circuit circ){
    TensorNetwork network;
    std::vector<size_t> last_used(circ.num_qubits);
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        network.tensors.push_back(TensorInfo{.type=INPUT,.io_label=bit,.op=NULL_OP});
        network.forward_edges.push_back();
        last_used[bit] = bit;
    }
    for(const GateInfo & gate : circ.gates){
        assert(gate.bit1 != NULL_BIT);
        size_t cur_idx = tensors.size();
        if(gate.bit2 == NULL_BIT){
            network.forward_edges.at(last_used.at(gate.bit1)).push_back(cur_idx);
            last_used.at(gate.bit1) = cur_idx;
        }
        else{
            network.forward_edges.at(last_used.at(gate.bit1)).push_back(cur_idx);
            network.forward_edges.at(last_used.at(gate.bit2)).push_back(cur_idx);
            last_used.at(gate.bit1) = cur_idx;
            last_used.at(gate.bit2) = cur_idx;
        }
        network.tensors.push_back(TensorInfo{.type=GATE,.io_label=NULL_IO_LAB,.op=NULL_OP});
        network.forward_edges.push_back();
    }
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        network.forward_edges.at(last_used.at(i)).push_back(cur_idx);
        network.tensors.push_back(TensorInfo{.type=OUTPUT,.io_label=bit,.op=NULL_OP});
        network.forward_edges.push_back();
    }
    return network;
}
void edge_cut(std::vector<TensorNetwork> & out_networks,std::vector<TensorNetwork> & in_networks){
    
}
std::vector<TensorNetwork> decompose_network(const TensorNetwork & in_network,std::vector<size_t> & partitioning){
    std::vector<TensorNetwork> networks = {in_network};

}
Circuit to_circuit(TensorNetwork network,std::vector<size_t> input_mapping){
    Circuit circ;

}
