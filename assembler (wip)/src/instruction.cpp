#include "../instruction.hpp"
#include <cstdint>
#include <limits>

enum class Flags : uint8_t
{
    BRA_NOT = 0x1,
    BRA_LT = 0x2,
    BRA_U = 0x4,
    BRA_EQ = 0x8,

    JMP_WORD = 0x1,
    JMP_IMM = 0x2,
    
    MEM_LOAD = 0x1,
    MEM_WORD = 0x2,
    MEM_SEX = 0x4,
};

uint8_t get_bra_cond_flags(Opcode opcode) {
    switch (opcode)
    {
    case Opcode::BEQ:  return *Flags::BRA_EQ;
    case Opcode::BNE:  return *Flags::BRA_EQ | *Flags::BRA_NOT;
    case Opcode::BLT:  return *Flags::BRA_LT;
    case Opcode::BLE:  return *Flags::BRA_LT | *Flags::BRA_EQ;
    case Opcode::BGT:  return *Flags::BRA_LT | *Flags::BRA_EQ | *Flags::BRA_NOT;
    case Opcode::BGE:  return *Flags::BRA_LT | *Flags::BRA_NOT;
    case Opcode::BLTU: return *Flags::BRA_U | *Flags::BRA_LT;
    case Opcode::BLEU: return *Flags::BRA_U | *Flags::BRA_LT | *Flags::BRA_EQ;
    case Opcode::BGTU: return *Flags::BRA_U | *Flags::BRA_LT | *Flags::BRA_EQ | *Flags::BRA_NOT;
    case Opcode::BGEU: return *Flags::BRA_U | *Flags::BRA_LT | *Flags::BRA_NOT;
    default:           return *Flags::BRA_NOT;
    }
}

uint8_t get_mem_flags(Opcode opcode) {
    switch (opcode)
    {
    case Opcode::LDW: return *Flags::MEM_LOAD | *Flags::MEM_WORD;
    case Opcode::LDB: return *Flags::MEM_LOAD | *Flags::MEM_SEX;
    case Opcode::LBU: return *Flags::MEM_LOAD;
    case Opcode::STW: return *Flags::MEM_WORD;
    case Opcode::STB: return 0;
    default:          return 0;
    }
}

bool Signature::operator==(const Signature& other) const {
    return opcode == other.opcode && arguments == other.arguments;
}

size_t std::hash<Signature>::operator()(const Signature& sig) const {
    size_t hash = _opcode_hasher(sig.opcode);
    for (const std::type_index& type: sig.arguments) {
        hash += _type_hasher(type);
        hash = (hash << 3) | (hash >> (std::numeric_limits<size_t>::digits - 3));
    }
    return hash;
}

const std::unordered_map<Signature, InstructionDef> INSTRUCTIONS {
    
};