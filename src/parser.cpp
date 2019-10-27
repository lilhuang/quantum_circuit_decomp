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
    return reg == EMPTY_REGISTER ? "E" : std::to_string(reg);
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
