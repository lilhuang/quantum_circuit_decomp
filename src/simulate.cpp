#include <fstream>

# include <stdio.h>
# include <math.h>
#include <iostream>

#include "simulate.h"
# include "QuEST.h"
#include "parser.h"

std::vector<float> simulate(std::string filename){
    std::ifstream file(filename);
    Circuit circuit = parseGates(file);
    int numQubits = circuit.num_qubits;
    file >> numQubits;

    // prepare QuEST
    QuESTEnv env = createQuESTEnv();

    // create qureg; let zeroth qubit be ancilla
    Qureg qureg = createQureg(numQubits, env);
    initZeroState(qureg);

    for(GateInfo gate : circuit.gates){
        switch(gate.op.gate){
            case GateTy::H:
                hadamard(qureg, gate.bit1);
                break;
            case GateTy::CNOT:
                controlledNot(qureg,gate.bit1,gate.bit2);
                break;
            case GateTy::Rz:
                rotateZ(qureg,gate.bit1,gate.op.rotation);
                break;
            case GateTy::Ry:
                rotateY(qureg,gate.bit1,gate.op.rotation);
                break;
            case GateTy::Rx:
                rotateX(qureg,gate.bit1,gate.op.rotation);
                break;
            default:
                throw std::runtime_error("gate not implemented!");
                break;
        }
    }
    //reportState(qureg);

    destroyQureg(qureg, env);
    destroyQuESTEnv(env);
    std::cout << "finished " << filename << "\n";
    return std::vector<float> {};
}
