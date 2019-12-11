# include <stdio.h>
# include <math.h>
#include <iostream>
#include <string>
#include <bitset>
#include <fstream>
#include <cmath>
#include <thread>
#include <random>

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

CircuitProbs mags_to_probs(std::vector<double> & circuit_mags){
    CircuitProbs probs;
    for(size_t i = 0; i < circuit_mags.size(); i++){
        QuantumFinalOut outbits(i);
        if(circuit_mags[i] != 0.0){
            probs[outbits] = circuit_mags[i];
        }
    }
    return probs;
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
double sqr(double x){
    return x * x;
}
double similarity(const CircuitProbs & c1,const std::vector<double> & c2){
    double sum = 0;
    for(const auto & keyval_pair : c1){
        uint64_t idx = keyval_pair.first.to_ullong();
        double P = keyval_pair.second;
        double Q = c2[idx];
        sum += sqr(Q-P);//P * log(P/Q);
    }
    return sqrt(sum);
}
void simulate_pauli_output(Qureg & qureg,int bit,int pauli){
    switch(pauli){
        case 0: break;// identity
        case 1: hadamard(qureg, bit);break;//x gate
        case 2: hadamard(qureg, bit); rotateZ(qureg,bit,3*(M_PI/2));break;//y gate
        case 3: break;//z gate
    }
}
void simulate_pauli_input(Qureg & qureg,int bit,int pauli){
    switch(pauli){
        case 0: break;// identity
        case 1: hadamard(qureg, bit);break;//x gate
        case 2: rotateZ(qureg,bit,(M_PI/2));hadamard(qureg, bit);break;//y gate
        case 3: break;//z gate
    }
}
void set_input_bit(Qureg & qureg, int bit, int val){
    if(val == 1){
        pauliX(qureg, bit);
    }
    //std::cout << measure(qureg,bit) << " ";
}
/*
using PresampledCirc = std::vector<std::vector<std::vector<qcomplex>>>;
void calculate_presampled_circuit(QuESTEnv & env,const MultiCircuit & m,PresampledCirc & presampled){
    constexpr size_t NUM_PAULI = 8;
    for(size_t circ = 0; circ < m.circuits.size(); circ++){

        const Circuit & cur_circ = m.circuits[circ];
        presampled.push_back(std::vector<std::vector<qcomplex>>(cur_circ.num_qubits));
        Qureg qureg = createQureg(cur_circ.num_qubits, env);
        const size_t pauli_max = (NUM_PAULI)<<(m.num_classical_registers);
        for(size_t idx = 0; idx < pauli_max; idx++){
            initZeroState(qureg);
            for(int qr = 0; qr < m.input_registers[circ].size(); qr++){
                size_t creg = m.input_registers[circ][qr];
                if(creg != EMPTY_REGISTER){
                    int classical_val = class_reg_vals[creg];
                    int pauli_val = pauli_choices[3*creg] + 2*int(pauli_choices[3*creg+1]);
                    int sign = pauli_val == 0 ? 1 : (pauli_choices[3*creg+2] ? -1 : 1);
                    final_sign *= sign;
                    set_input_bit(qureg,qr,classical_val);
                    simulate_pauli_input(qureg,qr,pauli_val);
                }
            }

            simulate_circuit_helper(qureg,cur_circ);

            for(int qr = 0; qr < m.output_registers[circ].size(); qr++){
                if(m.output_types[circ][qr] == OutputType::REGISTER_OUT){
                    size_t creg = m.output_registers[circ][qr];

                    int pauli_val = pauli_choices[3*creg]+2*int(pauli_choices[3*creg+1]);
                    simulate_pauli_output(qureg,qr,pauli_val);
                }
            }
        }
        destroyQureg(qureg, env);
    }
}*/
size_t sample_count = 0;
size_t one_count = 0;
void sample_multicirc_once(QuESTEnv & env,const MultiCircuit & m,uint64_t idx, uint64_t isx,CircuitSamples & measure_counts){
    std::bitset<64> pauli_choices(idx);
    std::bitset<64> class_reg_vals(isx);
    QuantumFinalOut final_out_bits;
    std::bitset<64> actual_reg_assign;
    int final_sign = 1;
    //std::vector<std::unordered_map<uint64_t,QuantumFinalOut>> circ_maps(m.circuits.size());
    for(size_t circ = 0; circ < m.circuits.size(); circ++){
        const Circuit & cur_circ = m.circuits[circ];
        Qureg qureg = createQureg(cur_circ.num_qubits, env);
        initZeroState(qureg);
        for(int qr = 0; qr < m.input_registers[circ].size(); qr++){
            size_t creg = m.input_registers[circ][qr];
            if(creg != EMPTY_REGISTER){
                //int classical_val = class_reg_vals[creg];
                int pauli_val = pauli_choices[3*creg] + 2*int(pauli_choices[3*creg+1]);
                int sign = pauli_val == 0 ? 1 : (pauli_choices[3*creg+2] ? -1 : 1);
                final_sign *= sign;
                sample_count++;
                int paul_c_choice = pauli_choices[3*creg+2];
                one_count += int(pauli_choices[3*creg+2]) == 1;
                set_input_bit(qureg,qr, pauli_choices[3*creg+2]);
                simulate_pauli_input(qureg,qr,pauli_val);
            }
        }

        simulate_circuit_helper(qureg,cur_circ);

        for(int qr = 0; qr < m.output_registers[circ].size(); qr++){
            if(m.output_types[circ][qr] == OutputType::REGISTER_OUT){
                size_t creg = m.output_registers[circ][qr];

                int pauli_val = pauli_choices[3*creg]+2*int(pauli_choices[3*creg+1]);
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
                size_t creg = m.output_registers[circ][qr];
                int pauli_val = pauli_choices[3*creg] + 2*int(pauli_choices[3*creg+1]);
                int sign = pauli_val == 0 ? 1 : (measure_val ? -1 : 1);
                final_sign *= sign;
                //actual_reg_assign[creg] = measure_val;
            }
        }
        destroyQureg(qureg, env);
    }
    //std::cout << one_count /double(sample_count) << " ";
    //if(actual_reg_assign == class_reg_vals){
        measure_counts[final_out_bits] += final_sign;
    //}
}
CircuitProbs samples_to_probs(CircuitSamples & samples,int num_samples,int qubits_communi){
    CircuitProbs probs;
    for(auto & kv_pair  : samples){
        double prob = (kv_pair.second*double(1LL<<(qubits_communi*2)))/num_samples;
        probs[kv_pair.first] = prob;
    }
    return probs;
}
void perform_sample(CircuitSamples * _measure_counts,const MultiCircuit * _m,size_t number_samples){
    QuESTEnv env = createQuESTEnv();
    constexpr size_t NUM_PAULI = 8;
    const MultiCircuit & m = *_m;
    CircuitSamples & measure_counts = *_measure_counts;
    measure_counts.clear();

    std::random_device rand_device;
    std::default_random_engine generator(rand_device());

    auto lrand = [&](size_t max){
        return std::uniform_int_distribution<size_t>(0,max-1)(generator);
    };
    //std::cout << lrand(2) << "\n";
    //std::cout << lrand(2) << "\n";
    //std::cout << lrand(2) << "\n";
    //std::cout << lrand(2) << "\n";

    const size_t pauli_max = (1ULL)<<(m.num_classical_registers*3);
    const size_t value_max = (1ULL << m.num_classical_registers);
    for(int i = 0; i < number_samples; i++){
        size_t pauli_val = lrand(pauli_max);
        size_t binary_val = lrand(value_max);
        sample_multicirc_once(env,m,pauli_val,binary_val,measure_counts);
    }
    destroyQuESTEnv(env);
}
void add_circ_samples(CircuitSamples & dest,CircuitSamples & src){
    for(auto samp_pair : src){
        dest[samp_pair.first] += samp_pair.second;
    }
}
CircuitProbs sampled_simulate_multicircuit(const MultiCircuit & m,size_t number_samples){
    unsigned num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector<CircuitSamples> samples(num_threads);
    size_t tot_samples = 0;
    for(int i = 0; i < num_threads; i++){
        size_t num_samples = i == num_threads-1 ? number_samples-tot_samples : number_samples/num_threads;
        threads.push_back(std::thread(perform_sample,&samples[i],&m,num_samples));
        tot_samples += num_samples;
    }
    CircuitSamples final_result;
    for(int i = 0; i < num_threads; i++){
        threads[i].join();
        add_circ_samples(final_result,samples[i]);
    }
    return samples_to_probs(final_result,number_samples,m.num_classical_registers);
}
