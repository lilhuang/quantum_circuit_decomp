#pragma once
#include <vector>
#include <cstddef>
#include <functional>

std::vector<size_t> calculate_best_partition(const std::vector<std::vector<size_t>> & directed, std::function<double(std::vector<size_t>)> evaluate);
