# include <stdio.h>
# include <math.h>
#include <iostream>
#include <string>
#include <bitset>
#include <fstream>
#include <cmath>


#include "simulate.h"
# include "QuEST.h"

void simulate_circuit_helper(Qureg & qureg,const Circuit & circuit){
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
}
std::vector<qcomplex> exact_simulate_circuit(const Circuit & circuit){
    int numQubits = circuit.num_qubits;

    // prepare QuEST
    QuESTEnv env = createQuESTEnv();

    // create qureg; let zeroth qubit be ancilla
    Qureg qureg = createQureg(numQubits, env);
    initZeroState(qureg);

    simulate_circuit_helper(qureg,circuit);
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
void simulate_pauli_output(Qureg & qureg,int bit,int pauli){
    switch(pauli){
        case 0: break;// identity
        case 1: hadamard(qureg, bit);break;//x gate
        case 2: rotateZ(qureg,bit,3*(M_PI/2));hadamard(qureg, bit);break;//y gate
        case 3: break;//z gate
    }
}
void simulate_pauli_input(Qureg & qureg,int bit,int pauli){
    switch(pauli){
        case 0: break;// identity
        case 1: hadamard(qureg, bit);break;//x gate
        case 2: hadamard(qureg, bit);rotateZ(qureg,bit,3*(M_PI/2));break;//y gate
        case 3: break;//z gate
    }
}
void set_input_bit(Qureg & qureg, int bit, int val){
    if(val == 1){
        pauliX(qureg, bit);
    }
}
CircuitSamples sampled_simulate_multicircuit(const MultiCircuit & m){
    QuESTEnv env = createQuESTEnv();
    constexpr size_t NUM_PAULI = 4;
    std::unordered_map<QuantumFinalOut, int> measure_counts;
    for(uint64_t idx = 0; idx < (NUM_PAULI)<<(m.num_classical_registers); idx++){
        std::bitset<64> pauli_choices(idx);
        for(uint64_t isx = 0; isx < (1ULL << m.num_classical_registers); isx++){
            std::bitset<64> class_reg_vals(isx);
            QuantumFinalOut final_out_bits;
            std::bitset<64> actual_reg_assign;
            for(size_t circ = 0; circ < m.circuits.size(); circ++){
                const Circuit & cur_circ = m.circuits[circ];
                Qureg qureg = createQureg(cur_circ.num_qubits, env);
                initZeroState(qureg);
                for(int qr = 0; qr < m.input_registers[circ].size(); qr++){
                    size_t creg = m.input_registers[circ][qr];
                    if(creg != EMPTY_REGISTER){
                        int classical_val = class_reg_vals[qr];
                        int pauli_val = pauli_choices[2*creg] + 2*int(pauli_choices[2*creg+1]);
                        set_input_bit(qureg,qr,classical_val);
                        simulate_pauli_input(qureg,qr,pauli_val);
                    }
                }

                simulate_circuit_helper(qureg,cur_circ);

                for(int qr = 0; qr < m.output_registers[circ].size(); qr++){
                    if(m.output_types[circ][qr] == OutputType::REGISTER_OUT){
                        size_t creg = m.output_registers[circ][qr];

                        int pauli_val = pauli_choices[2*creg]+2*int(pauli_choices[2*creg+1]);
                        simulate_pauli_output(qureg,qr,pauli_val);
                    }
                }
                for(int qr = 0; qr < m.output_registers[circ].size(); qr++){
                    int measure_val = measure(qureg,qr);
                    if(m.output_types[circ][qr] == OutputType::FINAL_OUT){
                        size_t final_out_qbit = m.output_registers[circ][qr];
                        final_out_bits[final_out_qbit] = measure_val;
                    }
                    else if(m.output_types[circ][qr] == OutputType::REGISTER_OUT){
                        size_t reg_out_bit = m.output_registers[circ][qr];
                        actual_reg_assign[reg_out_bit] = measure_val;
                    }
                }
            }
            if(actual_reg_assign == class_reg_vals){
                measure_counts[final_out_bits]++;
            }
        }
    }
}
