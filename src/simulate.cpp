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
        res[i] = (qcomplex(getRealAmp(qureg,i),getImagAmp(qureg,i)));
    }
    //std::vector<qcomplex> res = read_quest_reported_csv();
    //delete_reported_csv();

    destroyQureg(qureg, env);
    destroyQuESTEnv(env);
    return res;
}
CircuitSamples true_samples(const Circuit & circuit,int num_samples){
    int numQubits = circuit.num_qubits;

    // prepare QuEST
    QuESTEnv env = createQuESTEnv();
    // create qureg; let zeroth qubit be ancilla
    Qureg qureg = createQureg(numQubits, env);
    CircuitSamples samps;
    for(int i = 0; i < num_samples; i++){
        initZeroState(qureg);
        simulate_circuit_helper(qureg,circuit);
        //reportState(qureg);
        QuantumFinalOut fin_out;
        for(int qb = 0; qb < circuit.num_qubits; qb++){
            int measure_val = measure(qureg,qb);
            fin_out[qb] = measure_val;
        }
        samps[fin_out]++;
    }
    destroyQureg(qureg, env);
    destroyQuESTEnv(env);
    return samps;
}
size_t total_samples(const CircuitSamples & samps){
    size_t count = 0;
    for(const auto & keyval_pair : samps){
        count += keyval_pair.second;
    }
    return count;
}
double mag(qcomplex c){
    return (c * c).real();
}
std::vector<double> probability_mags(const std::vector<qcomplex> & exact_result){
    std::vector<double> res(exact_result.size());
    for(size_t i = 0; i < exact_result.size(); i++){
        res[i] = mag(exact_result[i]);
    }
    return res;
}

double similarity(const CircuitSamples & c1,const CircuitSamples & c2){
    //calculates Bhattacharyya distance
    size_t count1 = total_samples(c1);
    size_t count2 = total_samples(c2);

    double BC = 0;
    for(const auto & keyval_pair : c1){
        if(c2.count(keyval_pair.first)){
            BC += sqrt((keyval_pair.second/double(count1))*(c2.at(keyval_pair.first)/double(count2)));
        }
    }
    return -log(BC);
}
double similarity(const CircuitSamples & c1,const std::vector<double> & c2){
    size_t count1 = total_samples(c1);

    double sum = 0;
    for(const auto & keyval_pair : c1){
        uint64_t idx = keyval_pair.first.to_ullong();
        double P = keyval_pair.second/double(count1);
        double Q = c2[idx];
        sum += P * log(P/Q);
    }
    return sum;
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
    for(uint64_t converg_iters = 0; converg_iters < 40*(1ULL << m.num_classical_registers); converg_iters++){
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
    return measure_counts;
}
