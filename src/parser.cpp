#include "parser.h"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cassert>

Circuit parseGates(std::istream & input){
    Circuit circuit;
    input >> circuit.num_qubits;
    while(input){
        std::string line;
        std::getline(input, line);
        if(!line.size() || line[0] == '#'){
            continue;
        }

        std::istringstream reader(line);
        std::string name;
        reader >> name;
        GateInfo gate = {.op=OpInfo{.gate=GateTy::NULL_GATE,.rotation=NULL_INFO},
                        .bit1=NULL_BIT,.bit2=NULL_BIT};

        if(name == "H"){
            gate.op.gate = GateTy::H;
            reader >> gate.bit1;
        }
        else if(name == "X"){
            gate.op.gate = GateTy::X;
            reader >> gate.bit1;
        }
        else if(name == "Y"){
            gate.op.gate = GateTy::Y;
            reader >> gate.bit1;
        }
        else if(name == "Z"){
            gate.op.gate = GateTy::Z;
            reader >> gate.bit1;
        }
        else if(name == "CNOT"){
            gate.op.gate = GateTy::CNOT;
            reader >> gate.bit1 >> gate.bit2;
        }
        else if(name == "Rx"){
            gate.op.gate = GateTy::Rx;
            reader >> gate.op.rotation >> gate.bit1;
        }
        else if(name == "Ry"){
            gate.op.gate = GateTy::Ry;
            reader >> gate.op.rotation >> gate.bit1;
        }
        else if(name == "Rz"){
            gate.op.gate = GateTy::Rz;
            reader >> gate.op.rotation >> gate.bit1;
        }
        else{
            std::cout << name << std::endl;
            throw std::runtime_error("gate not implemented(parsing)!");
        }

        circuit.gates.push_back(gate);

        if(gate.bit1 != NULL_BIT){
            circuit.num_qubits = std::max(circuit.num_qubits,gate.bit1);

            if(gate.bit2 != NULL_BIT){
                circuit.num_qubits = std::max(circuit.num_qubits,gate.bit2);
            }
        }
    }
    return circuit;
}
std::string gate_name(OpInfo op){
    switch(op.gate){
        case GateTy::NULL_GATE:return "NULL_GATE";
        case GateTy::H:return "H";
        case GateTy::X:return "X";
        case GateTy::Y:return "Y";
        case GateTy::Z:return "Z";
        case GateTy::CNOT:return "CNOT";
        case GateTy::Rx:return "Rx " + std::to_string(op.rotation);
        case GateTy::Ry:return "Ry " + std::to_string(op.rotation);
        case GateTy::Rz:return "Rz " + std::to_string(op.rotation);
        default: throw std::runtime_error("gate not implemented(printing)!");
    }
}
void printGates(Circuit circuit,std::ostream & output){
    output << circuit.num_qubits << "\n";

    for(GateInfo & gate : circuit.gates){
        std::string name = gate_name(gate.op);
        int nbits = num_bits(gate.op.gate);
        if(nbits == 1){
            output << name << ' ' << gate.bit1 << "\n";
        }
        else if (nbits == 2){
            output << name << ' ' << gate.bit1 << ' ' << gate.bit2 << "\n";
        }
        else{
            throw std::runtime_error("gate not implemented(printing)!");
        }
    }
}

std::string reg_to_str(size_t reg){
    return reg == EMPTY_REGISTER ? "E" : "R"+std::to_string(reg);
}

std::string reg_to_str(size_t reg,OutputType type){
    if(type != OutputType::NULL_OUT){
        assert(reg != EMPTY_REGISTER);
    }
    if(type == OutputType::NULL_OUT){
        return "E";
    }
    else if(type == OutputType::REGISTER_OUT){
        return "R"+std::to_string(reg);
    }
    else if(type == OutputType::FINAL_OUT){
        return "F"+std::to_string(reg);
    }
    assert(false);
}
void printMultiCircuit(MultiCircuit multi_circ,std::ostream & output){
    for(size_t part = 0; part < multi_circ.circuits.size(); part++){
        output << "#circuit " << part << "\n";
        for(size_t in_r : multi_circ.input_registers[part]){
            output << reg_to_str(in_r) << ' ';
        }
        output << '\n';
        printGates(multi_circ.circuits[part],output);

        for(size_t ri = 0; ri < multi_circ.output_registers.at(part).size(); ri++){
            size_t out_reg = multi_circ.output_registers[part][ri];
            OutputType out_ty = multi_circ.output_types[part][ri];
            output << reg_to_str(out_reg,out_ty) << ' ';
        }
        output << "\n";
    }
}
void print_header(std::ostream & os){
    os << "digraph G {\n"
            "\trankdir=LR;\n"
            "\tnode [\n"
        "\tlabeljust=\"l\"\n"
        "\tcolorscheme=\"accent8\"\n"
        "\tstyle=filled\n"
        "\tshape=record\n"
    "]\n"
    "edge[\n"
        "penwidth=2\n"
    "]\n";
}
void print_footer(std::ostream & os){
    os << "}\n";
}
void print_node(std::ostream & os,std::string name, int node, int part){
    os << " n" << node << " [fillcolor="<<part+1<<", label=\""<<name<<"\"]\n";
}
void print_edge(std::ostream & os,int n1, int n2,bool diff_part){
    std::string color = diff_part ? "red" : "black";
    os << " n" << n1 << " -> " << "n" << n2 <<" [color="<<color<<"]" << "\n";
}
std::string tensor_name(TensorInfo info){
    switch(info.type){
        case TensorTy::GATE: return gate_name(info.op);
        case TensorTy::CONSTANT: return "∣0⟩";
        case TensorTy::FINAL_OUTPUT: return "∡";
        default: throw std::runtime_error("tensor type not implemented(tensor_name)!");
    }
}
void start_same_rank(std::ostream & os){
    os << "{\nrank=same;\n";
}
void end_same_rank(std::ostream & os){
    os << "}\n";
}
void print_nodes_of_type(TensorNetwork tn, std::vector<size_t> partitioning,TensorTy ty,std::ostream & os){
    for(size_t i = 0; i < tn.size(); i++){
        if(tn.tensors[i].type == ty){
            print_node(os,tensor_name(tn.tensors[i]),i,partitioning[i]);
        }
    }
}
void printPartitioning(TensorNetwork tn, std::vector<size_t> partitioning, std::ostream & os){
    print_header(os);

    start_same_rank(os);
    print_nodes_of_type(tn,partitioning,TensorTy::CONSTANT,os);
    end_same_rank(os);
    print_nodes_of_type(tn,partitioning,TensorTy::GATE,os);
    start_same_rank(os);
    print_nodes_of_type(tn,partitioning,TensorTy::FINAL_OUTPUT,os);
    end_same_rank(os);
    for(size_t n = 0; n < tn.size(); n++){
        for(size_t e : tn.forward_edges[n]){
            bool diff_part = partitioning[n] != partitioning[e];
            print_edge(os,n,e,diff_part);
        }
    }
    print_footer(os);
}
