#include "genetic_partitioning.h"
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <cassert>
#include <random>

using Info = std::vector<size_t>;
using Population = std::vector<Info>;

constexpr size_t POP_SIZE = 50;
constexpr size_t MUTATE_COUNT = 50;
constexpr size_t CROSSOVER_COUNT = 30;
constexpr size_t GENERATIONS = 200;
std::random_device rand_device;
std::default_random_engine generator(rand_device());
size_t lrand(){
    return std::uniform_int_distribution<size_t>()(generator);
}
void normalize_info(Info & info);
Population initialize(size_t graph_size){
    size_t init_decomp_size = graph_size/3;
    Population pop(POP_SIZE);
    for(Info & item : pop){
        item.resize(graph_size);
        for(size_t & v : item){
            v = lrand()%init_decomp_size;
        }
        normalize_info(item);
    }
    return pop;
}
size_t count_distinct(const Info & info){
    return std::unordered_set<size_t>(info.begin(),info.end()).size();
}
size_t max_el(const Info & info){
    return *std::max_element(info.begin(),info.end());
}
void normalize_info(Info & info){
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
}
size_t rand_sample_density_count(const std::vector<size_t> & densities){
    std::discrete_distribution<size_t> distribution(densities.begin(),densities.end());
    return distribution(generator);
}
Info crossover(const Info & v0,const Info & v1){
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
    normalize_info(res);
    return res;
}
Info mutate(const Info & v0){
    Info res = v0;
    int mutate_const = lrand()%2;
    int mutate_count = mutate_const ? lrand()%7 : lrand()%((v0.size()+15)/16);
    for(int i = 0; i < mutate_count; i++){
        res[lrand()%v0.size()] = v0[lrand()%v0.size()];
    }
    normalize_info(res);
    return res;
}
const Info & rand_choice(const Population & pop){
    return pop[lrand()%pop.size()];
}
/*
using Evals = std::vector<double>;
double sum(const Evals & e){
    return std::accumulate(e.begin(),e.end(),0.0);
}
double mean(const Evals & e){
    assert(e.size() >= 1);
    return sum(e)/e.size();
}
double sqr(double x){
    return x * x;
}
double variance(const Evals & e){
    assert(e.size() >= 2);
    double c = 0;
    double m = mean(e);
    for(double v : e){
        c += sqr(v-m);
    }
    return c / (e.size()-1);
}
double stdev(const Evals & e){
    return sqrt(variance(e));
}
void normalize(Evals & e){
    double s = 1.0/(sum(e)+1e-10);
    for(double & v : e){
        v *= s;
    }
}
Evals softmax(const Evals & e){
    double mean_v = mean(e);
    double stdev_v = stdev(e);
    std::vector<double> softmax_probs = e;
    for(double & v : softmax_probs){
        v = exp((v - mean_v)/stdev_v);
    }
    normalize(softmax_probs);
    return e;
}
size_t rand_sample_density_count(const std::vector<double> & densities){
    const double DISC_FACTOR = 10000000;
    std::vector<size_t> vec(densities.size());
    for(int i = 0; i < vec.size(); i++){
        vec[i] = size_t(densities[i]*DISC_FACTOR);
    }
    std::discrete_distribution<size_t> distribution(vec.begin(),vec.end());
    return distribution(generator);
}*/
Population compete(const Population & old_pop,size_t new_pop_size,std::function<double(std::vector<size_t>)> evaluate){
    if(old_pop.size() < new_pop_size){
        return old_pop;
    }
    struct CmpIdx{
        double x;
        size_t idx;
        bool operator < (CmpIdx o)const{return x > o.x;}
    };
    std::vector<CmpIdx> evaluations(old_pop.size());
    for(size_t i = 0; i < old_pop.size(); i++){
        evaluations[i] = CmpIdx{.x=evaluate(old_pop[i]),.idx=i};
    }
    std::sort(evaluations.begin(),evaluations.end());
    Population res_pop;
    for(size_t i = 0; i < new_pop_size; i++){
        CmpIdx evaledIdx = evaluations[i];
        res_pop.push_back(old_pop[evaledIdx.idx]);
    }
    /*std::vector<double> softmax_probs = softmax(evaluations);
    size_t add_count = 0;
    std::vector<char> added(evaluations.size(),false);
    Population res_pop;
    while(add_count < POP_SIZE){
        size_t add_idx = rand_sample_density_count(softmax_probs);
        if(!added[add_idx]){
            added[add_idx] = true;
            res_pop.push_back(old_pop[add_idx]);
            add_count++;
        }
    }*/
    return res_pop;
}
std::vector<size_t> calculate_best_partition(size_t graph_size, std::function<double(std::vector<size_t>)> evaluate){
    Population cur_pop = initialize(graph_size);
    for(int gen = 0; gen < GENERATIONS; gen++){
        Population child_pop = cur_pop;
        for(size_t i = 0; i < MUTATE_COUNT; i++){
            child_pop.emplace_back(mutate(rand_choice(cur_pop)));
        }
        for(size_t i = 0; i < CROSSOVER_COUNT; i++){
            child_pop.emplace_back(crossover(rand_choice(cur_pop),rand_choice(cur_pop)));
        }
        Population next_pop = compete(child_pop,POP_SIZE,evaluate);
        cur_pop.swap(next_pop);
    }
    Population final_el = compete(cur_pop,1,evaluate);
    return final_el.at(0);
}
