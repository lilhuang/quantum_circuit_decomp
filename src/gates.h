#pragma once
#include <cinttypes>

enum class GateTy{NULL_GATE,H,CNOT,Rx,Ry,Rz};
struct TensorOpInfo{
    GateTy gate;
    double rotation;
};
constexpr uint64_t NULL_BIT = uint64_t(-1);
constexpr double NULL_INFO = -1e50;
