#pragma once
#include <cinttypes>
#include <stdexcept>

enum class GateTy{NULL_GATE,H,X,Y,Z,CNOT,Rx,Ry,Rz};
struct OpInfo{
    GateTy gate;
    double rotation;
};
constexpr double NULL_INFO = -1e50;
constexpr OpInfo NULL_OP = OpInfo{.gate=GateTy::NULL_GATE,.rotation=NULL_INFO};
inline bool is_2bit_gate(GateTy gate){
    switch (gate) {
        case GateTy::H:
        case GateTy::X:
        case GateTy::Y:
        case GateTy::Z:
        case GateTy::Rx:
        case GateTy::Ry:
        case GateTy::Rz:
            return false;
        case GateTy::CNOT:
            return true;
        default:
            throw std::runtime_error("not valid gate");
    }
}
