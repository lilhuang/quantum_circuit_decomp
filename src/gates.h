#pragma once
#include <cinttypes>
#include <cstddef>

enum class GateTy{NULL_GATE,H,X,Y,Z,CNOT,Rx,Ry,Rz};
struct OpInfo{
    GateTy gate;
    double rotation;
};
constexpr double NULL_INFO = -1e50;
constexpr OpInfo NULL_OP = OpInfo{.gate=GateTy::NULL_GATE,.rotation=NULL_INFO};
inline size_t num_bits(GateTy gate){
    switch (gate) {
        case GateTy::H:
        case GateTy::X:
        case GateTy::Y:
        case GateTy::Z:
        case GateTy::Rx:
        case GateTy::Ry:
        case GateTy::Rz:
            return 1;
        case GateTy::CNOT:
            return 2;
        case GateTy::NULL_GATE:
            return 0;
    }
}
