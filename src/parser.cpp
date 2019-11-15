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
void printGates(Circuit circuit,std::ostream & output){
    output << circuit.num_qubits << "\n";

    for(GateInfo & gate : circuit.gates){
        switch(gate.op.gate){
            case GateTy::H:
                output << "H " << gate.bit1 << "\n";
                break;
            case GateTy::X:
                output << "X " << gate.bit1 << "\n";
                break;
            case GateTy::Y:
                output << "Y " << gate.bit1 << "\n";
                break;
            case GateTy::Z:
                output << "Z " << gate.bit1 << "\n";
                break;
            case GateTy::CNOT:
                output << "CNOT " << gate.bit1 << " "  << gate.bit2 << "\n";
                break;
            case GateTy::Rz:
                output << "Rz " << gate.op.rotation << " " << gate.bit1 << "\n";
                break;
            case GateTy::Ry:
                output << "Ry " << gate.op.rotation << " "  << gate.bit1 << "\n";
                break;
            case GateTy::Rx:
                output << "Rx " << gate.op.rotation << " "  << gate.bit1 << "\n";
                break;
            default:
                throw std::runtime_error("gate not implemented(printing)!");
                break;
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
const std::vector<std::string> part_colors = {
    "blue",
    "green",
    "red",
    "gray",
    "brown"
};
void print_input(std::ostream & os,int qbit){
    os << "\\node[draw=black] at (-0.31,"<<qbit<<") (I"<<qbit<<") {$|0\\rangle$};\n";
}

void print_output(std::ostream & os,int qbit,int pos){
    os << "\\node at ("<<pos<<"+0.31,"<<qbit<<") (O"<<qbit<<") {\\[\\Qcircuit {\\meter }\\]};\n";
}
void print_rectangle(std::ostream & os,int pos,int qbit1,int qubit2,int part,std::string name){
    os << "\\draw [draw=black,fill="<<part_colors.at(part)<<"] "
       <<"("<<pos<<",-0.4+"<<qbit1<<") rectangle "
       <<"(1+"<<pos<<",+0.4+"<<qubit2<<")"
       <<"node[pos=0.5] {\\footnotesize "<<name<<"};\n";
}
void print_circuit_line(std::ostream & os,int qubit,int pos_start,int pos_end,bool part_diffed){
    os << "\\draw ("<<pos_start<<","<<qubit<<") -- " <<
                  "("<<pos_end<<","<<qubit<<");\n";
}

void print_partition_tikz(Circuit circ,std::vector<size_t> partition, std::ostream & os){
    std::vector<size_t> positions(circ.num_qubits,0);
    size_t partition_offset = circ.num_qubits;
    os << partition.size() << "\n";
    os << partition.size() << "\n";
    os << partition.size() << "\n";
    for(size_t qbit = 0; qbit <circ.num_qubits; qbit++){
        print_input(os,qbit);
        positions[qbit]++;
    }
    for(size_t gidx = 0; gidx < circ.gates.size(); gidx++){
        GateInfo gate = circ.gates[gidx];
        if(num_bits(gate.op.gate) == 1){
            assert(gate.bit1 != NULL_BIT);
            print_rectangle(os,positions[gate.bit1],gate.bit1,gate.bit1,partition.at(gidx+partition_offset),"");

            print_circuit_line(os,gate.bit1,positions[gate.bit1]-1,positions[gate.bit1],false);

            positions[gate.bit1] += 2;
        }
        else{
            assert(gate.bit1 != NULL_BIT);
            assert(gate.bit2 != NULL_BIT);
            int gatepos = std::max(positions[gate.bit1],positions[gate.bit2]);
            print_rectangle(os,gatepos,gate.bit1,gate.bit2,partition.at(gidx+partition_offset),"");

            print_circuit_line(os,gate.bit1,positions[gate.bit1]-1,gatepos,false);
            print_circuit_line(os,gate.bit2,positions[gate.bit2]-1,gatepos,false);

            positions[gate.bit1] = gatepos+2;
            positions[gate.bit2] = gatepos+2;
        }
    }
    for(size_t qbit = 0; qbit < circ.gates.size(); qbit++){

    }
}
