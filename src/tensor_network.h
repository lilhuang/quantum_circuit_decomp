#pragma once
#include "gates.h"
#include <vector>
#include <cassert>
#include <unordered_map>
#include "circuit.h"

enum class TensorTy{GATE,CONSTANT,INPUT,OUTPUT,FINAL_OUTPUT};
constexpr size_t NULL_IO_LAB = size_t(-1);

struct TensorInfo{
    TensorTy type;
    size_t io_label;
    OpInfo op;
};
struct TensorNetwork{
    std::vector<TensorInfo> tensors;
    std::vector<std::vector<size_t>> forward_edges;
    std::vector<std::vector<size_t>> backward_edges;
    void add_node(TensorInfo info){
        tensors.push_back(info);
        forward_edges.emplace_back();
        backward_edges.emplace_back();
    }
    void add_edge(size_t src,size_t dest){
        forward_edges.at(src).push_back(dest);
        backward_edges.at(dest).push_back(src);
    }
    size_t size()const{
        return tensors.size();
    }
    size_t get_edge_idx(size_t node,size_t edge){
        const std::vector<size_t> & edges = forward_edges.at(node);
        for(size_t i = 0; i < edges.size(); i++){
            if(edges[i] == edge){
                return i;
            }
        }
        assert(false && "could not find edge");
    }
    template<class fn_ty>
    void iter_edges(size_t node,fn_ty fn){
        for(size_t edge : forward_edges[node]){
            fn(edge);
        }
        for(size_t edge : backward_edges[node]){
            fn(edge);
        }
    }
};
TensorNetwork from_circuit(Circuit circ);
//enum class ConectionTy{TRUE_INPUT,};
constexpr size_t NULL_CON = size_t(-1);
struct Connector{
    size_t part=NULL_CON;
    size_t idx=NULL_CON;
};
struct MultiGraphNetwork{
    std::vector<std::vector<Connector>> forward_edges;
    std::vector<std::vector<Connector>> backward_edges;
    std::vector<TensorNetwork> nodes;
    void add_connection(Connector src,Connector dest){
        forward_edges[src.part].push_back(dest);
        backward_edges[dest.part].push_back(src);
    }
};
MultiGraphNetwork create_multi_graph_network(TensorNetwork network,const std::vector<size_t>& partition_idxs);
Circuit to_circuit(TensorNetwork network);
