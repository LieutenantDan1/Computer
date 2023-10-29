#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <unordered_map>

template <typename E>
requires std::is_enum_v<E>
auto operator*(const E& e) {
    return std::underlying_type_t<E>(e);
}

enum class Opcode : uint8_t
{
    ADD, // 0
    SUB, // 1
    RS0, // 2, reserved
    RS1, // 3, reserved
    RS2, // 4, reserved
    RS3, // 5, reserved
    RS4, // 6, reserved
    LSL, // 7
    LSR, // 8
    ASR, // 9
    XOR, // A
    OR,  // B
    AND, // C
    BRA, // D
    JMP, // E
    MEM, // F

    BEQ, // pseudo-instruction
    BNE, // pseudo-instruction
    BLT, // pseudo-instruction
    BLE, // pseudo-instruction
    BGT, // pseudo-instruction
    BGE, // pseudo-instruction
    BLTU, // pseudo-instruction
    BLEU, // pseudo-instruction
    BGTU, // pseudo-instruction
    BGEU, // pseudo-instruction
    JSR, // pseudo-instruction
    CALL, // pseudo-instruction
    RET, // pseudo-instruction
    LDW, // pseudo-instruction
    LDB, // pseudo-instruction
    LBU, // pseudo-instruction
    STW, // pseudo-instruction
    STB, // pseudo-instruction
    SNOP, // pseudo-instruction
    NOP, // pseudo-instruction
    LNOP, // pseudo-instruction
    LDI, // pseudo-instruction
    MOV, // pseudo-instruction
};

enum class Register : uint8_t
{
    R0,
    RA,
    SP,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    S0,
    S1,
    S2,
    S3
};

const extern std::unordered_map<std::string, Opcode> OPCODES;
const extern std::unordered_map<std::string, Register> REGISTERS;