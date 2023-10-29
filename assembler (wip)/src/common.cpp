#include "../common.hpp"

const extern std::unordered_map<std::string, Opcode> OPCODES {
    { "add", Opcode::ADD },
    { "sub", Opcode::SUB },
    { "lsl", Opcode::LSL },
    { "lsr", Opcode::LSR },
    { "asr", Opcode::ASR },
    { "xor", Opcode::XOR },
    { "or",  Opcode::OR  },
    { "and", Opcode::AND },
    { "bra", Opcode::BRA },
    { "jmp", Opcode::JMP },

    { "jsr", Opcode::JSR },
    { "call", Opcode::CALL }, 
    { "ret", Opcode::RET },
    { "bra", Opcode::BRA },
    { "beq", Opcode::BEQ },
    { "bne", Opcode::BNE },
    { "blt", Opcode::BLT },
    { "ble", Opcode::BLE },
    { "bgt", Opcode::BGT },
    { "bge", Opcode::BGE },
    { "bltu", Opcode::BLTU },
    { "bleu", Opcode::BLEU },
    { "bgtu", Opcode::BGTU },
    { "bgeu", Opcode::BGEU },
    { "ldw", Opcode::LDW },
    { "ldb", Opcode::LDB },
    { "lbu", Opcode::LBU },
    { "stw", Opcode::STW },
    { "stb", Opcode::STB },
    { "ldi", Opcode::LDI },
    { "mov", Opcode::MOV },
    { "snop", Opcode::SNOP },
    { "nop", Opcode::NOP },
    { "lnop", Opcode::LNOP },
};

const extern std::unordered_map<std::string, Register> REGISTERS {
    { "r0", Register::R0, },
    { "r1", Register::RA, },
    { "r2", Register::SP, },
    { "r3", Register::A0, },
    { "r4", Register::A1, },
    { "r5", Register::A2, },
    { "r6", Register::A3, },
    { "r7", Register::T0, },
    { "r8", Register::T1, },
    { "r9", Register::T2, },
    { "r10", Register::T3, },
    { "r11", Register::T4, },
    { "r12", Register::S0, },
    { "r13", Register::S1, },
    { "r14", Register::S2, },
    { "r15", Register::S3, },

    { "zero", Register::R0, },
    { "ra", Register::RA, },
    { "sp", Register::SP, },
    { "a0", Register::A0, },
    { "a1", Register::A1, },
    { "a2", Register::A2, },
    { "a3", Register::A3, },
    { "t0", Register::T0, },
    { "t1", Register::T1, },
    { "t2", Register::T2, },
    { "t3", Register::T3, },
    { "t4", Register::T4, },
    { "s0", Register::S0, },
    { "s1", Register::S1, },
    { "s2", Register::S2, },
    { "s3", Register::S3, },
};