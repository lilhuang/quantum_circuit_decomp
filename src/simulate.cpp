# include <stdio.h>
# include <math.h>
#include <iostream>
#include <string>
#include <fstream>

#include "simulate.h"
# include "QuEST.h"

std::vector<qcomplex> exact_simulate_circuit(const Circuit & circuit){
    int numQubits = circuit.num_qubits;

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
            case GateTy::X:
                pauliX(qureg, gate.bit1);
                break;
            case GateTy::Y:
                pauliY(qureg, gate.bit1);
                break;
            case GateTy::Z:
                pauliZ(qureg, gate.bit1);
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
    size_t res_size = size_t(1)<<numQubits;
    std::vector<qcomplex> res(res_size);
    for(size_t i = 0; i < res_size; i++){
        res.push_back(qcomplex(getRealAmp(qureg,i),getImagAmp(qureg,i)));
    }
    //std::vector<qcomplex> res = read_quest_reported_csv();
    //delete_reported_csv();

    destroyQureg(qureg, env);
    destroyQuESTEnv(env);
    return res;
}
std::vector<CircuitSample> sampled_simulate_multicircuit(const MultiCircuit & m, std::vector<size_t> function_decomp){
    
}
