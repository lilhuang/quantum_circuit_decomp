#pragma once
#include <vector>
#include <cstddef>
#include <functional>

std::vector<size_t> calculate_partitions(const std::vector<std::vector<size_t>> & directed_graph,size_t num_parts);
std::vector<size_t> calculate_best_working_partition(const std::vector<std::vector<size_t>> & directed_graph, std::function<bool(std::vector<size_t>)> works_fn);
std::vector<size_t> split_unconnected_components(const std::vector<std::vector<size_t>> & undirected_graph,const std::vector<size_t> & partitioning);
std::vector<std::vector<size_t>> undirected_from_directed(std::vector<std::vector<size_t>> directed_graph);
