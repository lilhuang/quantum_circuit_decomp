#include "graph_partition.h"
#include "metis.h"
#include <stdexcept>
#include <queue>

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
std::vector<size_t> split_unconnected_components(const std::vector<std::vector<size_t>> & undirected_graph,const std::vector<size_t> & partitioning){
    size_t orig_size = partitioning.size();
    std::vector<size_t> new_partitioning(orig_size);

    std::vector<char> has_collected(orig_size);
    size_t new_partition_idx = 0;
    while(true){
        constexpr size_t NULL_CON = size_t(-1);
        size_t start_loc = NULL_CON;
        for(size_t i = 0; i < orig_size; i++){
            if(!has_collected[i]){
                start_loc = i;
                break;
            }
        }
        if(start_loc == NULL_CON){
            break;
        }

        size_t partition_loc = partitioning[start_loc];
        std::queue<size_t> neighbors;
        neighbors.push(start_loc);
        std::vector<size_t> partition_nodes;
        while(neighbors.size()){
            size_t next_loc = neighbors.front();
            neighbors.pop();
            if(has_collected.at(next_loc)){
                continue;
            }
            has_collected[next_loc] = true;
            new_partitioning[next_loc] = new_partition_idx;
            partition_nodes.push_back(next_loc);

            for(size_t edge : undirected_graph[next_loc]){
                if(partitioning[edge] == partition_loc){
                    neighbors.push(edge);
                }
            }
        }
        new_partition_idx++;
    }
    return new_partitioning;
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
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
    options[METIS_OPTION_PTYPE] = METIS_PTYPE_RB;
    //options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY;
    options[METIS_OPTION_UFACTOR] = 900;// load imballance (1+x)/1000
    options[METIS_OPTION_NITER] = 100;
    options[METIS_OPTION_NCUTS] = 100;
    options[METIS_OPTION_MINCONN] = 0;
    options[METIS_OPTION_CONTIG] = 1;//try to make graph contiguous
    CheckError(METIS_PartGraphKway(
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
        options,//options
        &result_edgeweight,
        partition_idxs.data()
    ));
    std::vector<size_t> partitioning(partition_idxs.begin(),partition_idxs.end());
    return split_unconnected_components(undirected_graph,partitioning);
}
size_t search_for_best(const std::vector<std::vector<size_t>> & directed_graph, std::function<bool(std::vector<size_t>)>  works_fn){
    size_t part_max = directed_graph.size();
    size_t part_min = 1;
    while(part_min < part_max-1){
        size_t part_middle = (part_max + part_min) / 2;
        if(works_fn(calculate_partitions(directed_graph,part_middle))){
            part_max = part_middle;
        }
        else{
            part_min = part_middle;
        }
    }
    return part_max;
}
std::vector<size_t> calculate_best_working_partition(const std::vector<std::vector<size_t>> & directed_graph, std::function<bool(std::vector<size_t>)>  works_fn){
    return calculate_partitions(directed_graph,search_for_best(directed_graph,works_fn));
}
