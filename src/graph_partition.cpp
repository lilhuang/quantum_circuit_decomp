#include "graph_partition.h"
#include "metis.h"
#include <stdexcept>

struct CompressedStorageFormat{
    std::vector<idx_t> xadj;
    std::vector<idx_t> adjncy;
};
CompressedStorageFormat compute_CSR(std::vector<std::vector<size_t>> undirected_graph){
    CompressedStorageFormat res;
    idx_t counter = 0;
    for(std::vector<size_t> & edges : undirected_graph){
        res.xadj.push_back(counter);
        for(size_t e : edges){
            res.adjncy.push_back(e);
            counter++;
        }
    }
    res.xadj.push_back(counter);
    return res;
}
std::vector<std::vector<size_t>> undirected_from_directed(std::vector<std::vector<size_t>> directed_graph){
    std::vector<std::vector<size_t>> undirected_graph(directed_graph.size());
    for(size_t n = 0; n < directed_graph.size(); n++){
        for(size_t e : directed_graph[n]){
            undirected_graph.at(e).push_back(n);
            undirected_graph.at(n).push_back(e);
        }
    }
    return undirected_graph;
}
void CheckError(int errcode){
    switch(errcode){
        case METIS_OK:break;
        case METIS_ERROR_INPUT:throw std::runtime_error("METIS_ERROR_INPUT");
        case METIS_ERROR_MEMORY:throw std::runtime_error("METIS_ERROR_MEMORY");
        case METIS_ERROR:throw std::runtime_error("METIS_ERROR");
    }
}
std::vector<size_t> calculate_partitions(const std::vector<std::vector<size_t>> & directed_graph,size_t num_parts){
    std::vector<std::vector<size_t>> undirected_graph = undirected_from_directed(directed_graph);
    CompressedStorageFormat CSR_data = compute_CSR(undirected_graph);
    idx_t num_verticies = directed_graph.size();
    idx_t num_constr = 1;
    idx_t num_parts_ = num_parts;

    idx_t result_edgeweight;
    std::vector<idx_t> partition_idxs(directed_graph.size());
    CheckError(METIS_PartGraphRecursive(
        &num_verticies,
        &num_constr,
        CSR_data.xadj.data(),
        CSR_data.adjncy.data(),
        NULL,//vwgt
        NULL,//vsize
        NULL,//adjwgt
        &num_parts_,
        NULL,//tpwgts
        NULL,//ubvec
        NULL,//options
        &result_edgeweight,
        partition_idxs.data()
    ));
    return std::vector<size_t>(partition_idxs.begin(),partition_idxs.end());
}
