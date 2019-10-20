#include "tensor_network.h"
#include <algorithm>

TensorNetwork from_circuit(Circuit circ){
    TensorNetwork network;
    std::vector<size_t> last_used(circ.num_qubits);
    std::vector<size_t> bit_last_used(circ.num_qubits);
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        network.add_node(0,1,TensorInfo{.type=TensorTy::CONSTANT,.io_label=bit,.op=NULL_OP});
        last_used[bit] = bit;
        bit_last_used[bit] = 0;
    }
    for(const GateInfo & gate : circ.gates){
        assert(gate.bit1 != NULL_BIT);
        size_t cur_idx = network.size();
        size_t num_qubits = num_bits(gate.op.gate);
        network.add_node(num_qubits,num_qubits,TensorInfo{.type=TensorTy::GATE,.io_label=NULL_IO_LAB,.op=gate.op});
        assert((num_qubits == 1) == (gate.bit2 == NULL_BIT));
        assert((num_qubits == 2) == (gate.bit2 != NULL_BIT));
        if(gate.bit2 == NULL_BIT){
            network.add_edge(last_used.at(gate.bit1),bit_last_used.at(gate.bit1),cur_idx,0);
            last_used.at(gate.bit1) = cur_idx;
            bit_last_used.at(gate.bit1) = 0;
        }
        else{
            network.add_edge(last_used.at(gate.bit1),bit_last_used.at(gate.bit1),cur_idx,0);
            network.add_edge(last_used.at(gate.bit2),bit_last_used.at(gate.bit2),cur_idx,1);
            last_used.at(gate.bit1) = cur_idx;
            bit_last_used.at(gate.bit1) = 0;
            last_used.at(gate.bit2) = cur_idx;
            bit_last_used.at(gate.bit2) = 1;
        }
    }
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        size_t cur_idx = network.size();
        network.add_node(1,0,TensorInfo{.type=TensorTy::FINAL_OUTPUT,.io_label=bit,.op=NULL_OP});
        network.add_edge(last_used.at(bit),bit_last_used.at(bit),cur_idx,0);
    }
    return network;
}
MultiGraphNetwork create_multi_graph_network(TensorNetwork network,const std::vector<size_t>& partition_idxs){
    MultiGraphNetwork multi_network;
    size_t old_size = network.size();
    size_t num_partitions = 1+(*std::max_element(partition_idxs.begin(),partition_idxs.end()));

    std::vector<std::vector<size_t>> partition_nodes(num_partitions);
    multi_network.nodes.resize(num_partitions);
    multi_network.forward_edges.resize(num_partitions);
    multi_network.backward_edges.resize(num_partitions);

    for(size_t node = 0; node < old_size; node++){
        partition_nodes[partition_idxs[node]].push_back(node);
    }
    std::vector<Connector> new_nodes(network.size(),Connector{.part=NULL_CON,.idx=NULL_CON});
    //create tensor networks nodes
    for(size_t part = 0; part < num_partitions; part++){
        std::vector<size_t> nodes = partition_nodes[part];
        TensorNetwork & new_tn = multi_network.nodes[part];
        for(size_t node : nodes){
            new_nodes[node] = Connector{.part=part,.idx=new_tn.size()};
            new_tn.add_node(network.backward_edges[node].size(),
                network.forward_edges[node].size(),
                network.tensors[node]);
        }
    }
    //create the connections between the tensor-networks
    for(size_t node = 0; node < network.size(); node++){
        for(size_t edge : network.forward_edges[node]){
            Connector new_node = new_nodes[node];
            Connector new_edge = new_nodes[edge];
            size_t node_out_bit = network.get_backward_edge_idx(edge,node);
            size_t node_in_bit = network.get_forward_edge_idx(node,edge);
            if(new_node.part != new_edge.part){
                TensorNetwork & node_tensor_net = multi_network.nodes[new_node.part];
                size_t out_node_idx = node_tensor_net.size();
                size_t out_forward_connector = multi_network.forward_edges[new_node.part].size();
                node_tensor_net.add_node(1,0,TensorInfo{.type=TensorTy::OUTPUT,.io_label=out_forward_connector,.op=NULL_OP});
                node_tensor_net.add_edge(new_node.idx,node_in_bit,out_node_idx,0);

                TensorNetwork & edge_tensor_net = multi_network.nodes[new_edge.part];
                size_t in_node_idx = edge_tensor_net.size();
                size_t in_backward_connector = multi_network.backward_edges[new_edge.part].size();
                edge_tensor_net.add_node(0,1,TensorInfo{.type=TensorTy::INPUT,.io_label=in_backward_connector,.op=NULL_OP});
                edge_tensor_net.add_edge(in_node_idx,0,new_edge.idx,node_out_bit);

                Connector new_src = {.part=new_node.part,.idx=out_node_idx};
                Connector new_dest = {.part=new_node.part,.idx=in_node_idx};
                multi_network.forward_edges[new_src.part].push_back(new_dest);
                multi_network.backward_edges[new_dest.part].push_back(new_src);
            }
            else{
                TensorNetwork & local_tn = multi_network.nodes[new_node.part];
                local_tn.add_edge(new_node.idx,node_in_bit,new_edge.idx,node_out_bit);
            }
        }
    }
    return multi_network;
}
Circuit to_circuit(TensorNetwork network){
    Circuit circ;
    //std::vector<GateInfo> qubit_assignment(network.size(),GateInfo());
    size_t qubit_counter = 0;
    std::vector<std::vector<size_t>> qubit_node_assignment(network.size());
    //std::vector<size_t> qubit_bit_assignment(network.size(),NULL_CON);
    for(size_t i = 0; i < network.size(); i++){
        TensorTy type = network.tensors[i].type;
        if(type == TensorTy::INPUT || type == TensorTy::CONSTANT){
            //qubit_assignment[i].op = network.tensors[i].op;
            qubit_node_assignment[i] = std::vector<size_t>{qubit_counter};
            qubit_counter++;
        }
    }
    circ.num_qubits = qubit_counter;
    std::unordered_map<size_t,size_t> out_qubit_mapping;
    std::unordered_map<size_t,size_t> final_out_qubit_mapping;
    while(true){
        bool assigned = false;
        for(size_t i = 0; i < network.size(); i++){
            if(qubit_node_assignment[i].size() == 0){
                TensorTy type = network.tensors[i].type;
                size_t prevnode1 = network.backward_edges[i].at(0);
                size_t tn_bit = network.get_forward_edge_idx(prevnode1,i);
                if(!qubit_node_assignment[prevnode1].size()){
                    continue;
                }
                size_t prev_qubit1 = qubit_node_assignment[prevnode1].at(tn_bit);
                if(type == TensorTy::OUTPUT){
                    out_qubit_mapping[network.tensors[i].io_label] = prev_qubit1;
                    qubit_node_assignment[i] = std::vector<size_t>{prev_qubit1};
                }
                else if (type == TensorTy::FINAL_OUTPUT){
                    final_out_qubit_mapping[network.tensors[i].io_label] = prev_qubit1;
                    qubit_node_assignment[i] = std::vector<size_t>{prev_qubit1};
                }
                else if (type == TensorTy::GATE){
                    OpInfo op = network.tensors[i].op;
                    GateInfo gate;
                    gate.op = network.tensors[i].op;
                    gate.bit1 = prev_qubit1;
                    if(num_bits(op.gate) == 2){
                        size_t prevnode2 = network.backward_edges[i].at(1);
                        size_t tn_bit = network.get_forward_edge_idx(prevnode2,i);
                        if(qubit_node_assignment[prevnode2].size() == 0){
                            continue;
                        }
                        size_t prev_qubit2 = qubit_node_assignment[prevnode2].at(tn_bit);

                        gate.bit2 = prev_qubit2;
                        qubit_node_assignment[i] = std::vector<size_t>{prev_qubit1,prev_qubit2};
                    }
                    else{
                        qubit_node_assignment[i] = std::vector<size_t>{prev_qubit1};
                    }
                    circ.gates.push_back(gate);
                }
                else{
                    assert(false && "found bad tensorty");
                }
                assigned = true;
            }
        }
        if(!assigned){
            break;
        }
    }
    return circ;
}
