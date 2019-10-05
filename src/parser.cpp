#include "parser.h"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>

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
        GateInfo gate = {.op=TensorOpInfo{.gate=GateTy::NULL_GATE,.rotation=NULL_INFO},
                        .bit1=NULL_BIT,.bit2=NULL_BIT};

        if(name == "H"){
            gate.op.gate = GateTy::H;
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
