#pragma once
#include <vector>
#include <cstddef>
#include <functional>

std::vector<size_t> calculate_best_partition(size_t graph_size, std::function<double(std::vector<size_t>)> evaluate);
