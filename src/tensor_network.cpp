#include "tensor_network.h"
#include <algorithm>
#include <iostream>

TensorNetwork from_circuit(Circuit circ,std::vector<uint8_t> used_bits){
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
        last_used.at(bit) = cur_idx;
        bit_last_used.at(bit) = 0;
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
            size_t node_in_bit = network.get_forward_edge_idx(node,edge);
            size_t node_out_bit = network.get_backward_edge_idx(edge,node);
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
                Connector new_dest = {.part=new_edge.part,.idx=in_node_idx};
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
void get_circuit_info(const TensorNetwork & network, Circuit & circ,
        std::unordered_map<size_t,size_t> & out_qubit_mapping,
        std::unordered_map<size_t,size_t> & final_out_qubit_mapping,
        std::unordered_map<size_t,size_t> & input_qubit_mapping){

    size_t qubit_counter = 0;
    std::vector<std::vector<size_t>> qubit_node_assignment(network.size());
    //std::vector<size_t> qubit_bit_assignment(network.size(),NULL_CON);
    for(size_t i = 0; i < network.size(); i++){
        TensorTy type = network.tensors[i].type;
        if(type == TensorTy::INPUT || type == TensorTy::CONSTANT){
            //qubit_assignment[i].op = network.tensors[i].op;
            if(type == TensorTy::INPUT){
                input_qubit_mapping[network.tensors[i].io_label] = qubit_counter;
            }
            qubit_node_assignment[i] = std::vector<size_t>{qubit_counter};
            qubit_counter++;
        }
    }
    circ.num_qubits = qubit_counter;
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
}
Circuit to_circuit(TensorNetwork network){
    Circuit circ;
    std::unordered_map<size_t,size_t> out_qubit_mapping;
    std::unordered_map<size_t,size_t> final_out_qubit_mapping;
    std::unordered_map<size_t,size_t> input_qubit_mapping;

    get_circuit_info(network,circ,out_qubit_mapping,final_out_qubit_mapping,input_qubit_mapping);
    return circ;
}
bool network_only_uses_computed(const TensorNetwork & net,
        size_t node,
        const std::vector<char> & computed_input,
        std::vector<char> & has_visited){
    if(has_visited.at(node)){
        return true;
    }
    TensorInfo tens = net.tensors.at(node);
    if(tens.type == TensorTy::INPUT && !computed_input.at(node)){
        return false;
    }
    for(size_t back_edge : net.backward_edges.at(node)){
        if(!network_only_uses_computed(net,back_edge,computed_input,has_visited)){
            return false;
        }
    }
    has_visited.at(node) = true;
    return true;
}
std::vector<size_t> network_only_uses_computed(const TensorNetwork & net,
        const std::vector<size_t> & outputs,
        const std::vector<char> & computed_input){
    std::vector<size_t> valid_outputs;
    std::vector<char> has_visited(net.size());
    for(size_t out : outputs){
        if(network_only_uses_computed(net,out,computed_input,has_visited)){
            valid_outputs.push_back(out);
        }
    }
    return valid_outputs;
}
template<typename iter_ty>
void iter_multigraph(const MultiGraphNetwork & net,iter_ty fn){
    for(const auto & tens : net.nodes){
        for(const auto & item : tens.tensors){
            fn(item);
        }
    }
}
size_t num_output_qubits(const MultiGraphNetwork & net){
    int max_v = -2;
    iter_multigraph(net,[&](TensorInfo info){
        if(info.type == TensorTy::FINAL_OUTPUT){
            max_v = std::max(max_v,(int)info.io_label);
        }
    });
    return max_v+1;
}
MultiCircuit to_multi_circuit(MultiGraphNetwork graph_network){
    MultiCircuit multi_circ;
    std::vector<std::vector<size_t>> forward_reg_alocs(graph_network.forward_edges.size());
    std::vector<std::vector<size_t>> backward_reg_alocs(graph_network.backward_edges.size());
    std::vector<size_t> final_out_alocs(num_output_qubits(graph_network),NULL_CON);
    size_t & register_count = multi_circ.num_classical_registers;
    register_count=0;
    for (size_t part = 0; part < graph_network.nodes.size(); part++){
        forward_reg_alocs[part].assign(graph_network.forward_edges[part].size(),NULL_CON);
        backward_reg_alocs[part].assign(graph_network.backward_edges[part].size(),NULL_CON);
    }

    int updated_count = 2;
    do{
        updated_count--;

        for (size_t part = 0; part < graph_network.nodes.size(); part++){
            const TensorNetwork & cur_tensor = graph_network.nodes[part];
            std::vector<char> computable_input_nodes(cur_tensor.size());
            std::vector<size_t> output_nodes;
            //step 1: calculate output nodes that need to be to computed
            for(size_t n = 0; n < cur_tensor.size(); n++){
                TensorInfo ninfo = cur_tensor.tensors[n];
                if(ninfo.type == TensorTy::OUTPUT){
                    if(forward_reg_alocs[part].at(ninfo.io_label) == NULL_CON){
                        output_nodes.push_back(n);
                    }
                }
                else if(ninfo.type == TensorTy::FINAL_OUTPUT){
                    if(final_out_alocs.at(ninfo.io_label) == NULL_CON){
                        output_nodes.push_back(n);
                    }
                }
            }
            //step 2: calculate input nodes that can be computed
            for(size_t n = 0; n < cur_tensor.size(); n++){
                TensorInfo ninfo = cur_tensor.tensors[n];
                if(ninfo.type == TensorTy::INPUT){
                    if(backward_reg_alocs[part].at(ninfo.io_label) != NULL_CON){
                        computable_input_nodes[n] = true;
                    }
                }
            }
            //step 3: calculate which output nodes can be calculated from the input nodes
            std::vector<size_t> compute_outputs = network_only_uses_computed(cur_tensor,output_nodes,computable_input_nodes);
            if(compute_outputs.size() == 0){
                continue;
            }
            //compute new nodes that need to be calculated
            //std::vector<char> nodes_to_compute(cur_tensor.size());
            //for(size_t out : compute_outputs){
            //     bool worked = network_only_uses_computed(cur_tensor,out,computable_input_nodes,nodes_to_compute);
            //     assert(worked);
            // }
            //step 4: calculate new tensor network with unnecessary nodes removed, build circuit
            // std::vector<size_t> remapping(cur_tensor.size(),NULL_CON);
            // size_t new_node = 0;
            // TensorNetwork new_tn;
            // for(size_t i = 0; i < cur_tensor.size(); i++){
            //     if(nodes_to_compute[i]){
            //         remapping[i] = new_node;
            //         new_tn.tensors.push_back(cur_tensor.tensors[i]);
            //         new_tn.forward_edges.push_back(cur_tensor.forward_edges[i]);
            //         new_tn.backward_edges.push_back(cur_tensor.backward_edges[i]);
            //         new_node++;
            //     }
            // }
            // for(size_t n = 0; n < new_tn.size(); n++){
            //     new_tn.iter_edges(n,[&](size_t & e){
            //         e = remapping.at(e);
            //     });
            // }

            //step 5: update register allocation at the tensor level
            for(size_t output : compute_outputs){
                TensorInfo ninfo = cur_tensor.tensors.at(output);
                if(ninfo.type == TensorTy::OUTPUT){
                    forward_reg_alocs[part].at(ninfo.io_label) = register_count;

                    Connector back_con = graph_network.forward_edges[part].at(ninfo.io_label);
                    TensorInfo back_tensor = graph_network.nodes.at(back_con.part).tensors.at(back_con.idx);
                    assert(back_tensor.type == TensorTy::INPUT);
                    size_t output_idx = back_tensor.io_label;
                    assert(graph_network.backward_edges.at(back_con.part).at(output_idx).idx == output);
                    backward_reg_alocs.at(back_con.part).at(output_idx) = register_count;

                    register_count++;
                }
                else{
                    final_out_alocs.at(ninfo.io_label) = ninfo.io_label;
                }
            }
            //step 6: actually build the circuits
            bool all_worked = true;
            for(size_t output : output_nodes){
                TensorInfo ninfo = cur_tensor.tensors.at(output);
                if(ninfo.type == TensorTy::OUTPUT){
                    if(forward_reg_alocs[part].at(ninfo.io_label) == NULL_CON){
                        all_worked = false;
                    }
                }
                else{
                    if(final_out_alocs.at(ninfo.io_label) == NULL_CON){
                        all_worked = false;
                    }
                }
            }
            if(!all_worked){
                continue;
            }
            Circuit circ;
            std::unordered_map<size_t,size_t> out_qubit_mapping;
            std::unordered_map<size_t,size_t> final_out_qubit_mapping;
            std::unordered_map<size_t,size_t> input_qubit_mapping;

            get_circuit_info(cur_tensor,circ,out_qubit_mapping,final_out_qubit_mapping,input_qubit_mapping);
            std::vector<size_t> input_registers(circ.num_qubits,EMPTY_REGISTER);
            std::vector<size_t> output_registers(circ.num_qubits,EMPTY_REGISTER);
            std::vector<OutputType> output_types(circ.num_qubits,OutputType::NULL_OUT);
            for(auto in_pair : input_qubit_mapping){
                size_t io_label = in_pair.first;
                size_t qubit = in_pair.second;
                size_t reg_assigned = backward_reg_alocs[part].at(io_label);
                input_registers[qubit] = reg_assigned;
            }
            for(auto out_pair : out_qubit_mapping){
                size_t io_label = out_pair.first;
                size_t qubit = out_pair.second;
                size_t reg_assigned = forward_reg_alocs[part].at(io_label);
                output_registers.at(qubit) = reg_assigned;
                output_types.at(qubit) = OutputType::REGISTER_OUT;
            }
            for(auto fin_out_pair : final_out_qubit_mapping){
                size_t io_label = fin_out_pair.first;
                size_t qubit = fin_out_pair.second;
                output_registers.at(qubit) = io_label;
                output_types.at(qubit) = OutputType::FINAL_OUT;
            }
            multi_circ.input_registers.push_back(input_registers);
            multi_circ.output_registers.push_back(output_registers);
            multi_circ.output_types.push_back(output_types);
            multi_circ.circuits.push_back(circ);
            updated_count++;
        }
    }while(updated_count > 0);
    return multi_circ;
}
