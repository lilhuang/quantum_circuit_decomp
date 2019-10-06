#pragma once
#include <cinttypes>

enum class GateTy{NULL_GATE,H,X,Y,Z,CNOT,Rx,Ry,Rz};
struct OpInfo{
    GateTy gate;
    double rotation;
};
constexpr double NULL_INFO = -1e50;
constexpr OpInfo NULL_OP = OpInfo{.gate=GateTy::NULL_GATE,.rotation=NULL_INFO};
