#include "tensor_network.h"
#include <algorithm>

TensorNetwork from_circuit(Circuit circ){
    TensorNetwork network;
    std::vector<size_t> last_used(circ.num_qubits);
    for(size_t bit = 0; bit < circ.num_qubits; bit++){
        network.add_node(TensorInfo{.type=TensorTy::CONSTANT,.io_label=bit,.op=NULL_OP});
        last_used[bit] = bit;
    }
    for(const GateInfo & gate : circ.gates){
        assert(gate.bit1 != NULL_BIT);
        size_t cur_idx = network.size();
        network.add_node(TensorInfo{.type=TensorTy::GATE,.io_label=NULL_IO_LAB,.op=gate.op});
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
        size_t cur_idx = network.size();
        network.add_node(TensorInfo{.type=TensorTy::FINAL_OUTPUT,.io_label=bit,.op=NULL_OP});
        network.add_edge(last_used.at(bit),cur_idx);
    }
    return network;
}
MultiGraphNetwork create_multi_graph_network(TensorNetwork network,const std::vector<size_t>& partition_idxs){
    MultiGraphNetwork multi_network;
    size_t old_size = network.size();
    size_t num_partitions = 1+(*std::max_element(partition_idxs.begin(),partition_idxs.end()));

    std::vector<std::vector<size_t>> partition_nodes(num_partitions);
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
            new_tn.add_node(network.tensors[node]);
        }
    }
    //create internal connections between tensor-networks
    /*for(size_t part = 0; part < num_partitions; part++){
        for(size_t node : nodes){
            for(size_t edge : network.forward_edges[node]){
                size_t src_node = new_nodes[node].idx;
                size_t dest_node = new_nodes[edge].idx;
                if(partition_idxs[edge] == partition_idxs[node]){
                    new_tn.add_edge(src_node,dest_node);
                }
                else{
                    new_tn.forward_edges[src_node].push_back(NULL_CON);
                }
            }
        }
    }*/
    //create the connections between the tensor-networks
    for(size_t node = 0; node < network.size(); node++){
        for(size_t edge : network.forward_edges[node]){
            Connector new_node = new_nodes[node];
            Connector new_edge = new_nodes[edge];
            if(new_node.part != new_edge.part){
                TensorNetwork & node_tensor_net = multi_network.nodes[new_node.part];
                size_t out_node_idx = node_tensor_net.size();
                size_t out_forward_connector = multi_network.forward_edges[new_node.part].size();
                node_tensor_net.add_node(TensorInfo{.type=TensorTy::OUTPUT,.io_label=out_forward_connector,.op=NULL_OP});
                node_tensor_net.add_edge(new_node.idx,out_node_idx);

                TensorNetwork & edge_tensor_net = multi_network.nodes[new_edge.part];
                size_t in_node_idx = edge_tensor_net.size();
                size_t in_backward_connector = multi_network.backward_edges[new_edge.part].size();
                edge_tensor_net.add_node(TensorInfo{.type=TensorTy::INPUT,.io_label=in_backward_connector,.op=NULL_OP});
                edge_tensor_net.add_edge(in_node_idx,new_edge.idx);

                Connector new_src = {.part=new_node.part,.idx=out_node_idx};
                Connector new_dest = {.part=new_node.part,.idx=in_node_idx};
                multi_network.forward_edges[new_src.part].push_back(new_dest);
                multi_network.backward_edges[new_dest.part].push_back(new_src);
            }
            else{
                TensorNetwork & local_tn = multi_network.nodes[new_node.part];
                local_tn.add_edge(new_node.idx,new_edge.idx);
            }
        }
    }
    return multi_network;
}
Circuit to_circuit(TensorNetwork network){
    Circuit circ;
    std::vector<GateInfo> qubit_assignment(network.size(),GateInfo());
    size_t qubit_counter = 0;
    for(size_t i = 0; i < network.size(); i++){
        TensorTy type = network.tensors[i].type ;
        if(type == TensorTy::INPUT || type == TensorTy::CONSTANT){
            qubit_assignment[i].op = network.tensors[i].op;
            qubit_assignment[i].bit1 = qubit_counter;
            qubit_counter++;
        }
    }
    while(true){
        bool assigned = false;
        for(size_t i = 0; i < network.size(); i++){
            /*if(qubit_assignment[i] == NULL_BIT){
                for(size_t ei = 0; ei <  network.backward_edges[i].size(); ei++){
                    size_t edge = network.backward_edges[i][ei];
                    if(qubit_assignment.at(edge).bit1 != NULL_BIT){
                        //(is_2bit_gate()

                        //)){

                    }
                }
            }*/
        }
    }
}
