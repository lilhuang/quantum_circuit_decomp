#include "genetic_partitioning.h"
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <cassert>
#include <random>
#include "graph_partition.h"

using Info = std::vector<size_t>;
using Population = std::vector<Info>;
using Graph = std::vector<std::vector<size_t>>;

constexpr size_t POP_SIZE = 50;
constexpr size_t MUTATE_COUNT = 120;
constexpr size_t CROSSOVER_COUNT = 80;
constexpr size_t GENERATIONS = 200;
std::random_device rand_device;
std::default_random_engine generator(rand_device());
size_t lrand(){
    return std::uniform_int_distribution<size_t>()(generator);
}
void normalize_info(Info & info,const Graph & graph);
Population initialize(size_t graph_size, const Graph & graph){
    size_t init_decomp_size = 2;
    Population pop(POP_SIZE);
    for(Info & item : pop){
        item.resize(graph_size);
        for(size_t & v : item){
            v = lrand()%init_decomp_size;
        }
        normalize_info(item,graph);
    }
    return pop;
}
size_t count_distinct(const Info & info){
    return std::unordered_set<size_t>(info.begin(),info.end()).size();
}
size_t max_el(const Info & info){
    return *std::max_element(info.begin(),info.end());
}
void normalize_info(Info & info, const Graph & graph){
    /*
     * Makes sure that all numbers between 0 and the max are in the info vector
     * by reassigning numbers if necessary.
    */
    size_t count = count_distinct(info);
    size_t max_v = max_el(info);
    if(count != max_v){
        const size_t UNASSIGNED = -1;
        std::vector<size_t> reassignment(max_v+1,UNASSIGNED);
        size_t reassigned_val = 0;
        for(size_t & v : info){
            if(reassignment[v] == UNASSIGNED){
                reassignment[v] = reassigned_val;
                reassigned_val++;
            }
            v = reassignment[v];
        }
    }
    info = split_unconnected_components(graph,info);
}
size_t rand_sample_density_count(const std::vector<size_t> & densities){
    std::discrete_distribution<size_t> distribution(densities.begin(),densities.end());
    return distribution(generator);
}
Info crossover(const Info & v0,const Info & v1, const Graph & graph){
    assert(v0.size() == v1.size());
    size_t info_size = v0.size();
    Info res = v0;
    size_t v0_parts = max_el(v0);
    size_t v1_parts = max_el(v1);
    for(size_t cross_part = 0; cross_part < v1_parts; cross_part++){
        //choose to crossover segment from v1
        if(lrand()%2 == 0){
            std::vector<size_t> cross_count(v0_parts+1,size_t(0));
            for(size_t i = 0; i < info_size; i++){
                if(v1[i] == cross_part){
                    cross_count[v0[i]]++;
                }
            }
            size_t rand_i = rand_sample_density_count(cross_count);
            for(size_t i = 0; i < info_size; i++){
                if(v1[i] == cross_part){
                    res[i] = rand_i;
                }
            }
        }
    }
    normalize_info(res,graph);
    return res;
}
Info mutate(const Info & v0, const Graph & graph){
    Info res = v0;
    int mutate_const = lrand()%2;
    int mutate_count = mutate_const ? lrand()%7 : lrand()%((v0.size()+15)/16);
    for(int i = 0; i < mutate_count; i++){
        res[lrand()%v0.size()] = v0[lrand()%v0.size()];
    }
    normalize_info(res,graph);
    return res;
}
const Info & rand_choice(const Population & pop){
    return pop[lrand()%pop.size()];
}
size_t ident_count(const Info & i1, const Info & i2){
    size_t count = 0;
    for(size_t i = 0; i < i1.size(); i++){
        count += i1[i] == i2[i];
    }
    return count;
}
Population compete(const Population & old_pop,size_t new_pop_size,std::function<double(std::vector<size_t>)> evaluate){
    if(old_pop.size() < new_pop_size){
        return old_pop;
    }
    std::vector<double> evaluations(old_pop.size());
    for(size_t i = 0; i < old_pop.size(); i++){
        evaluations[i] = evaluate(old_pop[i]);
    }
    std::vector<char> remaining(old_pop.size(),true);
    size_t remain_count = old_pop.size();
    size_t total_sim_count = 0;
    size_t loop_count = 1;
    while(remain_count > new_pop_size){
        size_t n1 = lrand()%old_pop.size();
        size_t n2 = lrand()%old_pop.size();
        size_t sim_count = ident_count(old_pop[n1],old_pop[n2]);
        size_t worse_idx = evaluations[n1] < evaluations[n2] ? n1 : n2;

        double average_sim_coun = (total_sim_count/loop_count);
        if(false && sim_count < average_sim_coun){
            if(lrand()%5 == 0){
                remaining[worse_idx] = false;
                remain_count--;
            }
        }
        else{
            remaining[worse_idx] = false;
            remain_count--;
        }

        total_sim_count += sim_count;
        loop_count++;
    }
    Population res_pop;
    for(size_t i = 0; i < old_pop.size(); i++){
        if(remaining[i]){
            res_pop.emplace_back(old_pop[i]);
        }
    }
    return res_pop;
}
std::vector<size_t> calculate_best_partition(const std::vector<std::vector<size_t>> & directed, std::function<double(std::vector<size_t>)> evaluate){
    size_t graph_size = directed.size();
    Graph undirected = undirected_from_directed(directed);
    Population cur_pop = initialize(graph_size,undirected);

    for(int gen = 0; gen < GENERATIONS; gen++){
        Population child_pop = cur_pop;
        for(size_t i = 0; i < MUTATE_COUNT; i++){
            child_pop.emplace_back(mutate(rand_choice(cur_pop),undirected));
        }
        for(size_t i = 0; i < CROSSOVER_COUNT; i++){
            child_pop.emplace_back(crossover(rand_choice(cur_pop),rand_choice(cur_pop),undirected));
        }
        Population next_pop = compete(child_pop,POP_SIZE,evaluate);
        cur_pop.swap(next_pop);
    }
    Info best_info;
    double best_val = -1e100;
    for(size_t i = 0; i < cur_pop.size(); i++){
        double cur_val = evaluate(cur_pop[i]);
        if(best_val < cur_val){
            best_val = cur_val;
            best_info = cur_pop[i];
        }
    }
    //Population final_el = compete(cur_pop,1,evaluate);
    return best_info;
}
